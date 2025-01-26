#pragma once

#include <optional>
#include <utility>
//
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/json/object.hpp>

#include "api_handler.h"
#include "file_handler.h"
#include "utils/logger.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

namespace request_handler {
namespace beast = boost::beast;
namespace http = beast::http;

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
   public:
    using Strand = boost::asio::io_context::executor_type;
    using JsonResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;

    explicit RequestHandler(api_handler::ApiHandler& api_handler,
                            file_handler::FileHandler& file_handler)
        : api_handler_(api_handler), file_handler_(file_handler) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send) {
        BOOST_LOG_TRIVIAL(info) << "Hello8";
        using namespace std::literals;
        const auto response =
            [this, &req, &send](
                http::status status, std::string_view answer,
                boost::string_view content_type = ContentType::TEXT,
                std::optional<boost::string_view> cache_control = std::nullopt,
                std::optional<boost::string_view> allow = std::nullopt) {
                return send(MakeStringResponse(status, answer, req.version(),
                                               req.keep_alive(), content_type,
                                               cache_control, allow));
            };

        BOOST_LOG_TRIVIAL(info) << "Hello9";

        if (req.target().starts_with(api_handler::ApiHandler::API_KEY)) {
            return api_handler_(std::move(req), std::move(response));
        }

        BOOST_LOG_TRIVIAL(info) << "Hello10";

        if (req.method() == http::verb::get) {
            std::optional<FileResponse> file_response =
                ProcessGetFiles(req.target(), req.version(), req.keep_alive());

            BOOST_LOG_TRIVIAL(info) << "Hello12";

            if (!file_response) {
                BOOST_LOG_TRIVIAL(info) << "Hello13";
                return response(http::status::not_found, "File not found");
            }

            return send(*file_response);
        }

        BOOST_LOG_TRIVIAL(info) << "Hello11";
    }

   private:
    api_handler::ApiHandler& api_handler_;
    file_handler::FileHandler& file_handler_;

    std::optional<FileResponse> ProcessGetFiles(std::string_view target,
                                                unsigned http_version,
                                                bool keep_alive) const;

    JsonResponse MakeStringResponse(
        const http::status status, const std::string_view body,
        const unsigned http_version, const bool keep_alive,
        const boost::string_view content_type,
        const std::optional<boost::string_view> cache_control,
        const std::optional<boost::string_view> allow) const;
};

}  // namespace request_handler
