#include "api_handler.h"

#include "json_converter.h"

namespace api_handler {

ApiHandler::ApiHandler(app::Game& game) : game_(game) {}

std::string ApiHandler::GetMapsResponse() const {
    boost::json::array maps;
    for (const auto map : game_.GetMaps()) {
        maps.push_back(json_converter::MapToJson(map));
    }
    return boost::json::serialize(maps);
}

std::pair<http::status, std::string> ApiHandler::GetMapResponse(
    const MapId id) const {
    auto map_answer = TryFindMap(id);
    if (std::holds_alternative<MapNotFoundAnswer>(map_answer)) {
        auto not_found_answer = std::get<MapNotFoundAnswer>(map_answer);
        return std::make_pair(not_found_answer.first, not_found_answer.second);
    }

    auto map = std::get<ConstMapPointer>(map_answer);
    auto map_json = json_converter::FullMapToJson(*map);
    return std::make_pair(http::status::ok, boost::json::serialize(map_json));
}

std::variant<ApiHandler::MapNotFoundAnswer, ApiHandler::ConstMapPointer>
ApiHandler::TryFindMap(const model::Map::Id map_id) const {
    auto map = game_.FindMap(map_id);
    if (!map) {
        return std::make_pair(
            http::status::not_found,
            GetDefaultResponse("mapNotFound", "Map not found"));
    }
    return map;
}

std::string ApiHandler::GetDefaultResponse(const std::string code,
                                           const std::string message) const {
    return boost::json::serialize(
        boost::json::value{{"code", code}, {"message", message}});
}

}  // namespace api_handler
