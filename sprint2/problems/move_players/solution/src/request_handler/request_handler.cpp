#include "request_handler.h"

#include <string>
//
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/functional/hash.hpp>
#include <boost/json/serialize.hpp>

#include "request_handler/utils/error_codes.h"
#include "request_handler/utils/response_utils.h"

namespace request_handler {

std::variant<RequestHandler::JsonResponse, RequestHandler::FileResponse>
RequestHandler::ProcessGetFiles(http::verb method, boost::string_view target,
                                unsigned http_version, bool keep_alive) const {
    if (method != http::verb::get) {
        return ReportServerError(http_version, keep_alive);
    }

    auto file_response =
        (*file_handler_)(std::filesystem::path(std::string(target)));

    if (!file_response) {
        return MakeJsonResponse(
            http_version, keep_alive,
            response_utils::StringResponse{.status = http::status::not_found,
                                           .answer = "File not found"});
    }

    return MakeFileResponse(std::move(*file_response));
}

RequestHandler::JsonResponse RequestHandler::MakeJsonResponse(
    const unsigned http_version, const bool keep_alive,
    response_utils::StringResponse string_response) const {
    JsonResponse response(string_response.status, http_version);
    response.set(http::field::content_type, string_response.content_type);
    if (string_response.cache_control) {
        response.set(http::field::cache_control,
                     *string_response.cache_control);
    }
    if (string_response.allow) {
        response.set(http::field::allow, *string_response.allow);
    }
    response.body() = string_response.answer;
    response.content_length(string_response.answer.size());
    response.keep_alive(keep_alive);
    return response;
}

RequestHandler::FileResponse RequestHandler::MakeFileResponse(
    file_handler::FileResponse&& file_response) const {
    FileResponse response;
    response.body() = std::move(file_response.file);
    response.set(http::field::content_type,
                 content_type::ExtensionToContentType(file_response.extension));
    return response;
}

RequestHandler::JsonResponse RequestHandler::ReportServerError(
    const unsigned http_version, const bool keep_alive) const {
    return MakeJsonResponse(
        http_version, keep_alive,
        response_utils::MakeBadRequestResponse(error_codes::kBadRequest,
                                               "Internal server error"));
}

}  // namespace request_handler
