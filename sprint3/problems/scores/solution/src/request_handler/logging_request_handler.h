#pragma once
#include <memory>
#include <utility>

#include <boost/chrono.hpp>

#include "request_handler.h"
#include "utils/logger.h"

class LoggingRequestHandler {
    static void LogRequest(const boost::string_view client_ip,
                           const boost::string_view uri,
                           const boost::string_view method) {
        BOOST_LOG_TRIVIAL(info)
            << boost::log::add_value(
                   additional_data,
                   boost::json::value{
                       {"ip", client_ip}, {"URI", uri}, {"method", method}})
            << "request received";
    }
    template <typename Response>
    static void LogResponse(
        const Response& response,
        const boost::chrono::duration<double, boost::ratio<1, 1000>>
            elapsed_time) {
        BOOST_LOG_TRIVIAL(info)
            << boost::log::add_value(
                   additional_data,
                   boost::json::value{
                       {"response_time",
                        static_cast<int>(elapsed_time.count())},
                       {"code", response.result_int()},
                       {"content_type",
                        response[boost::beast::http::field::content_type]}})
            << "response sent";
    }

   public:
    LoggingRequestHandler(
        std::shared_ptr<request_handler::RequestHandler> decorated)
        : decorated_(decorated) {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(
        boost::string_view client_ip,
        request_handler::http::request<
            Body, request_handler::http::basic_fields<Allocator>>&& req,
        Send&& send) {
        auto start_time = boost::chrono::high_resolution_clock::now();
        auto logging_sender = [send = std::move(send),
                               start_time](auto&& response) {
            auto end_time = boost::chrono::high_resolution_clock::now();
            LoggingRequestHandler::LogResponse(response, end_time - start_time);
            send(response);
        };
        LogRequest(client_ip, req.target(),
                   boost::beast::http::to_string(req.method()));
        (*decorated_)(std::move(req), std::move(logging_sender));
    }

   private:
    std::shared_ptr<request_handler::RequestHandler> decorated_;
};
