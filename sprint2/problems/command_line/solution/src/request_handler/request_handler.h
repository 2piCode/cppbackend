#pragma once

#include <memory>
#include <utility>
#include <variant>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/json/object.hpp>

#include "api_handler/api_handler.h"
#include "file_handler.h"
#include "request_handler/utils/response_utils.h"
#include "utils/logger.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

namespace request_handler {
namespace beast = boost::beast;
namespace http = beast::http;

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
   public:
    using Strand = boost::asio::strand<boost::asio::io_context::executor_type>;
    using JsonResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;

    explicit RequestHandler(std::filesystem::path static_files_root,
                            app::Application::Pointer app_ptr, Strand strand,
                            bool is_aviable_game_tick)
        : api_strand_(strand),
          api_handler_(std::make_shared<api_handler::ApiHandler>(
              std::move(app_ptr), is_aviable_game_tick)),
          file_handler_(
              std::make_shared<file_handler::FileHandler>(static_files_root)) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send) {
        using namespace std::literals;
        auto version = req.version();
        auto keep_alive = req.keep_alive();
        auto target = req.target();

        try {
            if (target.starts_with(api_handler::ApiHandler::API_KEY)) {
                auto handle = [self = shared_from_this(), send,
                               req = std::forward<decltype(req)>(req), version,
                               keep_alive] {
                    try {
                        assert(self->api_strand_.running_in_this_thread());
                        return send(self->MakeJsonResponse(
                            version, keep_alive, (*self->api_handler_)(req)));
                    } catch (...) {
                        send(self->ReportServerError(version, keep_alive));
                    }
                };
                return boost::asio::dispatch(api_strand_, handle);
            }

            return std::visit(
                [&send](auto&& result) {
                    send(std::forward<decltype(result)>(result));
                },
                ProcessGetFiles(req.method(), target, version, keep_alive));

        } catch (...) {
            send(ReportServerError(version, keep_alive));
        }
    }

   private:
    Strand api_strand_;
    std::shared_ptr<api_handler::ApiHandler> api_handler_;
    std::shared_ptr<file_handler::FileHandler> file_handler_;

    std::variant<JsonResponse, FileResponse> ProcessGetFiles(
        http::verb method, boost::string_view target, unsigned http_version,
        bool keep_alive) const;

    JsonResponse MakeJsonResponse(const unsigned http_version,
                                  const bool keep_alive,
                                  response_utils::StringResponse) const;

    FileResponse MakeFileResponse(file_handler::FileResponse&& response) const;

    JsonResponse ReportServerError(const unsigned http_version,
                                   const bool keep_alive) const;
};
}  // namespace request_handler
