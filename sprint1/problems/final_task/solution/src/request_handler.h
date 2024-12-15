#pragma once
#include <cinttypes>
#include <optional>
#include <typeinfo>
#include <utility>

#include "boost/beast/http/status.hpp"
#include "boost/json/serialize.hpp"
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
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view JSON = "application/json"sv;
};

struct ContentPath {
    ContentPath() = delete;
    constexpr static std::string_view GET_MAPS = "/api/v1/maps";
};

}  // namespace detail

class RequestHandler {
   public:
    using JsonResponse = http::response<http::string_body>;

    explicit RequestHandler(model::Game& game) : game_{game} {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send) {
        using namespace std::literals;
        const auto text_response =
            [this, &req, &send](http::status status, std::string_view answer) {
                return send(MakeStringResponse(status, answer, req.version(),
                                               req.keep_alive()));
            };

        if (req.method() == http::verb::get) {
            if (req.target() == std::string(detail::ContentPath::GET_MAPS)) {
                return text_response(http::status::ok, GetMapsResponse());
            }

            auto start_it =
                req.target().find(std::string(detail::ContentPath::GET_MAPS));

            if (start_it != std::string::npos) {
                auto target_str = req.target().substr(
                    detail::ContentPath::GET_MAPS.size() + 1);
                auto [status_code, body] =
                    GetMapResponse(std::string(target_str));
                return text_response(status_code, body);
            }
        }

        return text_response(http::status::bad_request, GetBadResponse());
    }

   private:
    model::Game& game_;

    JsonResponse MakeStringResponse(
        const http::status status, const std::string_view body,
        const unsigned http_version, const bool keep_alive,
        const std::string_view content_type = detail::ContentType::JSON) const;

    std::string GetMapsResponse() const;

    std::pair<http::status, std::string> GetMapResponse(
        const std::string id) const;

    std::string GetBadResponse() const;

    boost::json::object GetDefaultResponse(const std::string code,
                                           const std::string message) const;
};

}  // namespace http_handler
