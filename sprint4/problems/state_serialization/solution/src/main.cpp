#include "utils/sdk.h"
//
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/archive/archive_exception.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/log/trivial.hpp>

#include "app/application.h"
#include "http_server/http_server.h"
#include "json_loader.h"
#include "request_handler/logging_request_handler.h"
#include "request_handler/request_handler.h"
#include "serialization/application_state.h"
#include "utils/command_line_parser.h"
#include "utils/logger.h"
#include "utils/ticker.h"

using namespace std::literals;
namespace net = boost::asio;

namespace {

template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

app::Application::Pointer CreateApplication(const utils::Args& args) {
    std::optional<app::Application::Pointer> app_ptr = std::nullopt;
    if (args.state_file) {
        app_ptr = LoadApplicationState(*args.state_file);
    }
    if (!app_ptr) {
        app_ptr = std::make_unique<app::Application>(
            std::make_shared<app::Players>(),
            json_loader::LoadGame(args.config_file),
            json_loader::LoadLootGenerator(args.config_file),
            json_loader::LoadLootHandler(args.config_file),
            json_loader::LoadNumberMapHandler(args.config_file),
            args.is_random_spawnpoint);
    }
    return std::move(*app_ptr);
}

void TrySaveApplicationState(app::Application::Pointer app_ptr,
                             std::optional<std::string> state_file) {
    if (!state_file) {
        return;
    }

    static std::filesystem::path temp_application_state_file("temp_file");
    SaveApplicationState(app_ptr, temp_application_state_file);
    std::filesystem::rename(temp_application_state_file, *state_file);
}

}  // namespace

int main(int argc, const char* argv[]) {
    std::optional<utils::Args> args;
    try {
        args = utils::ParseCommandLine(argc, argv);
        if (!args) {
            return EXIT_FAILURE;
        }

    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    try {
        InitBoostLogFilter();

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec,
                                  [[maybe_unused]] int signal_number) {
            if (!ec) {
                BOOST_LOG_TRIVIAL(info)
                    << boost::log::add_value(
                           additional_data,
                           boost::json::value{{"code", 0}, {"exception", ""}})
                    << "server exited";
                ioc.stop();
            }
        });

        app::Application::Pointer app_ptr = CreateApplication(*args);
        auto api_strand = net::make_strand(ioc);

        if (args->delta_time) {
            auto ticker = std::make_shared<utils::Ticker>(
                api_strand, *args->delta_time,
                [app = app_ptr](std::chrono::milliseconds delta) {
                    app->Tick(delta);
                });

            ticker->Start();
        }

        boost::signals2::scoped_connection save_state_connection =
            app_ptr->DoOnTick([total = 0ms, period = args->save_state_period,
                               state_file = args->state_file, &app_ptr](
                                  std::chrono::milliseconds delta) mutable {
                if (!period.has_value()) {
                    return;
                }
                total += delta;
                if (total >= *period) {
                    TrySaveApplicationState(app_ptr, state_file);
                    total = 0ms;
                }
            });

        auto handler = std::make_shared<request_handler::RequestHandler>(
            args->static_source_folder, app_ptr, api_strand,
            !args->delta_time.has_value());
        LoggingRequestHandler logging_handler{handler};

        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port},
                               [&logging_handler](const boost::string_view ip,
                                                  auto&& req, auto&& send) {
                                   logging_handler(
                                       ip, std::forward<decltype(req)>(req),
                                       std::forward<decltype(send)>(send));
                               });

        BOOST_LOG_TRIVIAL(info)
            << boost::log::add_value(
                   additional_data,
                   boost::json::value{{"port", port},
                                      {"address", address.to_string()}})
            << "server started";

        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });
        TrySaveApplicationState(app_ptr, args->state_file);
    } catch (const boost::archive::archive_exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Invalid data in state file";
    } catch (const std::exception& ex) {
        BOOST_LOG_TRIVIAL(error)
            << boost::log::add_value(
                   additional_data,
                   boost::json::value{{"code", EXIT_FAILURE},
                                      {"exception", ex.what()}})
            << "server exited";
        return EXIT_FAILURE;
    }
}
