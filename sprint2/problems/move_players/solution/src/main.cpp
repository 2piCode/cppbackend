#include "utils/sdk.h"
//
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include <vector>
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include "http_server/http_server.h"
#include "json_loader.h"
#include "request_handler/logging_request_handler.h"
#include "request_handler/request_handler.h"
#include "utils/logger.h"

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
    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <static-folder>"sv
                  << std::endl;
        return EXIT_FAILURE;
    }
    try {
        InitBoostLogFilter();

        app::Game::Pointer game = json_loader::LoadGame(argv[1]);

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        AddSignalsHandler(ioc);

        auto api_strand = net::make_strand(ioc);
        auto handler = std::make_shared<request_handler::RequestHandler>(
            argv[2], game, api_strand);
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
