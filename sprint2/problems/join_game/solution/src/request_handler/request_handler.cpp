#include "request_handler.h"

#include <string>
//
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/functional/hash.hpp>
#include <boost/json/serialize.hpp>

namespace request_handler {

std::optional<RequestHandler::FileResponse> RequestHandler::ProcessGetFiles(
    std::string_view target, unsigned http_version, bool keep_alive) const {
    FileResponse response;

    auto writer = [&response](http::file_body::value_type&& file,
                              std::string extension) {
        response.body() = std::move(file);
        response.set(http::field::content_type,
                     ExtensionToContentType(extension));
    };

    if (!file_handler_(std::filesystem::path(std::string(target)), writer)) {
        return std::nullopt;
    }

    return response;
}

RequestHandler::JsonResponse RequestHandler::MakeStringResponse(
    const http::status status, const std::string_view body,
    const unsigned http_version, const bool keep_alive,
    const boost::string_view content_type,
    const std::optional<boost::string_view> cache_control,
    const std::optional<boost::string_view> allow) const {
    JsonResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    if (cache_control) {
        response.set(http::field::cache_control, *cache_control);
    }
    if (allow) {
        response.set(http::field::allow, *allow);
    }
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

}  // namespace request_handler
