#include "utils/sdk.h"
//
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include "app/application.h"
#include "http_server/http_server.h"
#include "json_loader.h"
#include "request_handler/logging_request_handler.h"
#include "request_handler/request_handler.h"
#include "utils/command_line_parser.h"
#include "utils/logger.h"
#include "utils/ticker.h"

using namespace std::literals;
namespace net = boost::asio;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

void AddSignalsHandler(net::io_context& ioc) {
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

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

        app::Game::Pointer game = json_loader::LoadGame(args->config_file);

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        AddSignalsHandler(ioc);

        app::Application::Pointer app_ptr = std::make_unique<app::Application>(
            game, args->is_random_spawnpoint,
            json_loader::LoadLootGenerator(args->config_file),
            json_loader::LoadLootHandler(args->config_file),
            json_loader::LoadNumberMapHandler(args->config_file));

        auto api_strand = net::make_strand(ioc);

        if (args->delta_time) {
            auto ticker = std::make_shared<utils::Ticker>(
                api_strand, *args->delta_time,
                [&app = *app_ptr.get()](std::chrono::milliseconds delta) {
                    app.Tick(delta);
                });

            ticker->Start();
        }

        auto handler = std::make_shared<request_handler::RequestHandler>(
            args->static_source_folder, std::move(app_ptr), api_strand,
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
