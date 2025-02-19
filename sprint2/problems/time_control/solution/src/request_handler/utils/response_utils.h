#pragma once

#include <optional>
#include <string>
//
#include <boost/beast/http/status.hpp>
#include <boost/json.hpp>

#include "request_handler/utils/content_type.h"

namespace request_handler::response_utils {

namespace beast = boost::beast;
namespace http = beast::http;

struct StringResponse {
    http::status status;
    std::string answer;
    boost::string_view content_type = content_type::TEXT;
    std::optional<boost::string_view> cache_control = std::nullopt;
    std::optional<boost::string_view> allow = std::nullopt;
};

namespace detail {

static constexpr boost::string_view NO_CACHE_KEY = "no-cache";

inline std::string GetJsonResponse(std::string_view code,
                                   std::string_view message) {
    boost::json::object body{{"code", code}, {"message", message}};
    return boost::json::serialize(body);
}

}  // namespace detail

inline StringResponse MakeErrorResponse(boost::beast::http::status http_status,
                                        std::string_view error_code,
                                        std::string_view error_message) {
    return StringResponse{
        .status = http_status,
        .answer = detail::GetJsonResponse(error_code, error_message),
        .content_type = content_type::JSON,
        .cache_control = detail::NO_CACHE_KEY};
}

inline StringResponse MakeBadRequestResponse(std::string_view code,
                                             std::string_view message) {
    return MakeErrorResponse(boost::beast::http::status::bad_request, code,
                             message);
}

inline StringResponse MakeNotFoundResponse(std::string_view code,
                                           std::string_view message) {
    return MakeErrorResponse(boost::beast::http::status::not_found, code,
                             message);
}

inline StringResponse MakeUnauthorizedResponse(std::string_view code,
                                               std::string_view message) {
    return MakeErrorResponse(http::status::unauthorized, code, message);
}

inline StringResponse MakeMethodNotAllowedResponse(
    const std::string_view code, const boost::string_view allow_methods) {
    return StringResponse{
        .status = boost::beast::http::status::method_not_allowed,
        .answer = detail::GetJsonResponse(code, "Method not allowed"),
        .content_type = content_type::JSON,
        .cache_control = detail::NO_CACHE_KEY,
        .allow = allow_methods};
}

inline StringResponse MakeOkResponseCached(
    const boost::json::value& json_body) {
    return StringResponse{.status = boost::beast::http::status::ok,
                          .answer = boost::json::serialize(json_body),
                          .content_type = content_type::JSON};
}

inline StringResponse MakeOkResponse(const boost::json::value& json_body) {
    return StringResponse{.status = boost::beast::http::status::ok,
                          .answer = boost::json::serialize(json_body),
                          .content_type = content_type::JSON,
                          .cache_control = detail::NO_CACHE_KEY};
}

}  // namespace request_handler::response_utils
