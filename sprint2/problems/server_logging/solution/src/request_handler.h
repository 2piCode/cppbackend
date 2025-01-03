#pragma once
#include <cinttypes>
#include <optional>
#include <typeinfo>
#include <utility>

#include "boost/beast/http/status.hpp"
#include "file_handler.h"
#include "http_server.h"
#include "json_converter.h"
#include "model.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;

namespace detail {

using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static boost::string_view TEXT_HTML = "text/html";
    constexpr static boost::string_view JSON = "application/json";
    constexpr static boost::string_view CSS = "text/css";
    constexpr static boost::string_view TEXT = "text/plain";
    constexpr static boost::string_view JS = "text/javascript";
    constexpr static boost::string_view XML = "application/xml";
    constexpr static boost::string_view PNG = "image/png";
    constexpr static boost::string_view JPG = "image/jpeg";
    constexpr static boost::string_view GIF = "image/gif";
    constexpr static boost::string_view BMP = "image/bmp";
    constexpr static boost::string_view ICO = "image/vnd.microsoft.icon";
    constexpr static boost::string_view TIFF = "image/tiff";
    constexpr static boost::string_view SVG = "image/svg+xml";
    constexpr static boost::string_view MP3 = "audio/mpeg";
    constexpr static boost::string_view OCTET_STREAM =
        "application/octet-stream";
};

struct ContentPath {
    ContentPath() = delete;
    constexpr static boost::string_view GET_MAPS = "/api/v1/maps";
    constexpr static boost::string_view API_KEY = "/api";
};

}  // namespace detail

class RequestHandler {
   public:
    using JsonResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;

    explicit RequestHandler(model::Game& game, FileHandler& file_handler)
        : game_{game}, file_handler_(file_handler) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send) {
        using namespace std::literals;
        const auto text_response =
            [this, &req, &send](
                http::status status, std::string_view answer,
                boost::string_view content_type = detail::ContentType::TEXT) {
                return send(MakeStringResponse(status, answer, req.version(),
                                               req.keep_alive(), content_type));
            };

        if (req.method() == http::verb::get) {
            if (req.target().starts_with(detail::ContentPath::API_KEY)) {
                auto [status, answer] = ProcessApiPath(req.target());
                return text_response(status, answer, detail::ContentType::JSON);
            }

            std::optional<FileResponse> file_response =
                ProcessGetFiles(req.target(), req.version(), req.keep_alive());

            if (!file_response) {
                return text_response(http::status::not_found, "File not found");
            }

            return send(*file_response);
        }

        return text_response(http::status::method_not_allowed, "Invalid method",
                             detail::ContentType::TEXT_HTML);
    }

   private:
    model::Game& game_;
    FileHandler file_handler_;

    std::pair<http::status, std::string> ProcessApiPath(
        boost::string_view target) const;

    std::optional<FileResponse> ProcessGetFiles(boost::string_view target,
                                                unsigned http_version,
                                                bool keep_alive) const;

    JsonResponse MakeStringResponse(const http::status status,
                                    const std::string_view body,
                                    const unsigned http_version,
                                    const bool keep_alive,
                                    const boost::string_view content_type =
                                        detail::ContentType::TEXT) const;

    std::string GetMapsResponse() const;

    std::pair<http::status, std::string> GetMapResponse(
        const std::string id) const;

    std::string GetBadResponse() const;

    boost::json::object GetDefaultResponse(const std::string code,
                                           const std::string message) const;
};

}  // namespace http_handler
