#include "request_handler.h"

#include "boost/beast/http/fields.hpp"
#include "boost/beast/http/file_body.hpp"
#include "boost/functional/hash.hpp"
#include "boost/unordered_map.hpp"

namespace http_handler {

std::pair<http::status, std::string> RequestHandler::ProcessApiPath(
    boost::string_view target) const {
    if (target == std::string(detail::ContentPath::GET_MAPS)) {
        return std::make_pair(http::status::ok, GetMapsResponse());
    }

    auto start_it = target.find(detail::ContentPath::GET_MAPS);

    if (start_it != std::string::npos) {
        auto target_str =
            target.substr(detail::ContentPath::GET_MAPS.size() + 1);
        auto [status_code, body] = GetMapResponse(std::string(target_str));
        return std::make_pair(status_code, body);
    }

    return std::make_pair(http::status::bad_request, GetBadResponse());
}

boost::string_view ExtensionToContentType(boost::string_view extension) {
    static boost::unordered_map<boost::string_view, boost::string_view>
        extension_content_type{
            {".css", detail::ContentType::CSS},
            {".htm", detail::ContentType::TEXT_HTML},
            {".html", detail::ContentType::TEXT_HTML},
            {".txt", detail::ContentType::TEXT},
            {".js", detail::ContentType::JS},
            {".json", detail::ContentType::JSON},
            {".xml", detail::ContentType::XML},
            {".png", detail::ContentType::PNG},
            {".jpg", detail::ContentType::JPG},
            {".jpe", detail::ContentType::JPG},
            {".jpeg", detail::ContentType::JPG},
            {".gif", detail::ContentType::GIF},
            {".bmp", detail::ContentType::BMP},
            {".ico", detail::ContentType::ICO},
            {".tiff", detail::ContentType::TIFF},
            {".tif", detail::ContentType::TIFF},
            {".svg", detail::ContentType::SVG},
            {".svgz", detail::ContentType::SVG},
            {".mp3", detail::ContentType::MP3},
        };

    auto found_extension = extension_content_type.find(extension);
    if (found_extension == extension_content_type.end()) {
        return detail::ContentType::OCTET_STREAM;
    }
    return extension_content_type[extension];
}

std::optional<RequestHandler::FileResponse> RequestHandler::ProcessGetFiles(
    boost::string_view target, unsigned http_version, bool keep_alive) const {
    FileResponse response;

    auto writer = [&response](http::file_body::value_type&& file,
                              boost::string_view extension) {
        response.body() = std::move(file);
        std::cout << extension << std::endl;
        response.set(http::field::content_type,
                     ExtensionToContentType(extension));
    };

    if (!file_handler_.GetFile(std::filesystem::path(target.to_string()),
                               writer)) {
        return std::nullopt;
    }

    return response;
}

RequestHandler::JsonResponse RequestHandler::MakeStringResponse(
    const http::status status, const std::string_view body,
    const unsigned http_version, const bool keep_alive,
    const boost::string_view content_type) const {
    JsonResponse response(status, http_version);
    response.set(http::field::content_type, std::string(content_type));
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

std::string RequestHandler::GetMapsResponse() const {
    boost::json::array maps;
    for (const auto map : game_.GetMaps()) {
        maps.push_back(json_converter::MapToJson(map));
    }
    return boost::json::serialize(maps);
}

std::pair<http::status, std::string> RequestHandler::GetMapResponse(
    const std::string id) const {
    auto map = game_.FindMap(model::Map::Id{id});
    if (!map) {
        return std::make_pair(http::status::not_found,
                              boost::json::serialize(GetDefaultResponse(
                                  "mapNotFound", "Map not found")));
    }

    auto map_json = json_converter::FullMapToJson(*map);
    return std::make_pair(http::status::ok, boost::json::serialize(map_json));
}

std::string RequestHandler::GetBadResponse() const {
    return boost::json::serialize(
        GetDefaultResponse("badRequest", "Bad Request"));
}

boost::json::object RequestHandler::GetDefaultResponse(
    const std::string code, const std::string message) const {
    boost::json::object main;
    main["code"] = code;
    main["message"] = message;
    return main;
}

}  // namespace http_handler
