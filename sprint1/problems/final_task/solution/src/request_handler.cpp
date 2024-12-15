#include "request_handler.h"

namespace http_handler {

RequestHandler::JsonResponse RequestHandler::MakeStringResponse(
    const http::status status, const std::string_view body,
    const unsigned http_version, const bool keep_alive,
    const std::string_view content_type) const {
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
