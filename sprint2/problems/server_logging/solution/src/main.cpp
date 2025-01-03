#include "logging_request_handler.h"
#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "http_server.h"
#include "json_loader.h"
#include "logger.h"
#include "logging_request_handler.h"
#include "request_handler.h"

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

        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);
        FileHandler file_handler{argv[2]};

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        AddSignalsHandler(ioc);
        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        http_handler::RequestHandler handler{game, file_handler};
        LoggingRequestHandler logging_handler{handler};
        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику
        // запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;

        http_server::ServeHttp(ioc, {address, port},
                               [&logging_handler](const boost::string_view ip,
                                                  auto&& req, auto&& send) {
                                   logging_handler(
                                       ip, std::forward<decltype(req)>(req),
                                       std::forward<decltype(send)>(send));
                               });

        // Эта надпись сообщает тестам о том, что сервер запущен и готов
        // обрабатывать запросы

        BOOST_LOG_TRIVIAL(info)
            << boost::log::add_value(
                   additional_data,
                   boost::json::value{{"port", port},
                                      {"address", address.to_string()}})
            << "server started";

        // 6. Запускаем обработку асинхронных операций
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
