#include "json_converter.h"

#include <sys/poll.h>

#include <boost/utility/string_view.hpp>

#include "model.h"

struct MapFields {
    MapFields() = delete;
    constexpr static boost::string_view ID = "id";
    constexpr static boost::string_view NAME = "name";
    constexpr static boost::string_view ROADS = "roads";
    constexpr static boost::string_view BUILDINGS = "buildings";
    constexpr static boost::string_view OFFICES = "offices";
};

struct RoadFields {
    RoadFields() = delete;
    constexpr static boost::string_view X0 = "x0";
    constexpr static boost::string_view Y0 = "y0";
    constexpr static boost::string_view X1 = "x1";
    constexpr static boost::string_view Y1 = "y1";
};

struct BuildingFields {
    BuildingFields() = delete;
    constexpr static boost::string_view X = "x";
    constexpr static boost::string_view Y = "y";
    constexpr static boost::string_view W = "w";
    constexpr static boost::string_view H = "h";
};

struct OfficeFields {
    OfficeFields() = delete;
    constexpr static boost::string_view ID = "id";
    constexpr static boost::string_view X = "x";
    constexpr static boost::string_view Y = "y";
    constexpr static boost::string_view OFFSET_X = "offsetX";
    constexpr static boost::string_view OFFSET_Y = "offsetY";
};

boost::json::object json_converter::MapToJson(const model::Map& map) {
    boost::json::object main;

    main[MapFields::ID] = *map.GetId();
    main[MapFields::NAME] = map.GetName();

    return main;
}

boost::json::object json_converter::FullMapToJson(const model::Map& map) {
    auto main = MapToJson(map);

    main[MapFields::ROADS] = RoadsToJson(map.GetRoads());
    main[MapFields::BUILDINGS] = BuildingsToJson(map.GetBuildings());
    main[MapFields::OFFICES] = OfficesToJson(map.GetOffices());

    return main;
}

boost::json::array json_converter::RoadsToJson(
    const std::vector<model::Road>& roads) {
    boost::json::array roads_json;

    for (const auto& road : roads) {
        roads_json.push_back(RoadToJson(road));
    }
    return roads_json;
}

boost::json::object json_converter::RoadToJson(const model::Road& road) {
    boost::json::object main;

    main[RoadFields::X0] = road.GetStart().x;
    main[RoadFields::Y0] = road.GetStart().y;
    if (road.GetStart().x != road.GetEnd().x) {
        main[RoadFields::X1] = road.GetEnd().x;
    } else {
        main[RoadFields::Y1] = road.GetEnd().y;
    }

    return main;
}

boost::json::array json_converter::BuildingsToJson(
    const std::vector<model::Building>& buildings) {
    boost::json::array buildings_json;

    for (const auto& building : buildings) {
        buildings_json.push_back(BuildingToJson(building));
    }
    return buildings_json;
}

boost::json::object json_converter::BuildingToJson(
    const model::Building& building) {
    boost::json::object main;

    main[BuildingFields::X] = building.GetBounds().position.x;
    main[BuildingFields::Y] = building.GetBounds().position.y;
    main[BuildingFields::W] = building.GetBounds().size.width;
    main[BuildingFields::H] = building.GetBounds().size.height;

    return main;
}

boost::json::array json_converter::OfficesToJson(
    const std::vector<model::Office>& offices) {
    boost::json::array offices_json;

    for (const auto& office : offices) {
        offices_json.push_back(OfficeToJson(office));
    }

    return offices_json;
}

boost::json::object json_converter::OfficeToJson(const model::Office& office) {
    boost::json::object main;

    main[OfficeFields::ID] = *office.GetId();
    main[OfficeFields::X] = office.GetPosition().x;
    main[OfficeFields::Y] = office.GetPosition().y;
    main[OfficeFields::OFFSET_X] = office.GetOffset().dx;
    main[OfficeFields::OFFSET_Y] = office.GetOffset().dy;

    return main;
}

model::Map json_converter::JsonToMap(const json::object map_json) {
    model::Map map(
        model::Map::Id{map_json.at(MapFields::ID).as_string().c_str()},
        map_json.at(MapFields::NAME).as_string().c_str());

    return map;
}

std::vector<model::Road> json_converter::JsonToRoads(
    const boost::json::array roads_json) {
    std::vector<model::Road> roads;

    for (const auto& road_json : roads_json) {
        roads.push_back(JsonToRoad(road_json.as_object()));
    }

    return roads;
}

model::Road json_converter::JsonToRoad(const json::object road_json) {
    model::Point start{
        .x = static_cast<int>(road_json.at(RoadFields::X0).as_int64()),
        .y = static_cast<int>(road_json.at(RoadFields::Y0).as_int64())};

    if (const auto value = road_json.if_contains(RoadFields::X1)) {
        return model::Road(model::Road::HORIZONTAL, start,
                           static_cast<int>(value->as_int64()));
    }
    return model::Road(
        model::Road::VERTICAL, start,
        static_cast<int>(road_json.at(RoadFields::Y1).as_int64()));
}

std::vector<model::Building> json_converter::JsonToBuildings(
    const boost::json::array buildings_json) {
    std::vector<model::Building> buildings;

    for (const auto& building_json : buildings_json) {
        buildings.push_back(JsonToBuilding(building_json.as_object()));
    }

    return buildings;
}

model::Building json_converter::JsonToBuilding(
    const boost::json::object building_json) {
    return model::Building(model::Rectangle{
        .position =
            model::Point{.x = static_cast<int>(
                             building_json.at(BuildingFields::X).as_int64()),
                         .y = static_cast<int>(
                             building_json.at(BuildingFields::Y).as_int64())},
        .size =
            model::Size{.width = static_cast<int>(
                            building_json.at(BuildingFields::W).as_int64()),
                        .height = static_cast<int>(
                            building_json.at(BuildingFields::H).as_int64())}});
}

std::vector<model::Office> json_converter::JsonToOffices(
    const boost::json::array offices_json) {
    std::vector<model::Office> offices;
    for (const auto& office_json : offices_json) {
        offices.push_back(JsonToOffice(office_json.as_object()));
    }
    return offices;
}

model::Office json_converter::JsonToOffice(
    const boost::json::object office_json) {
    return model::Office(
        model::Office::Id{office_json.at(OfficeFields::ID).as_string().c_str()},
        model::Point{
            .x = static_cast<int>(office_json.at(OfficeFields::X).as_int64()),
            .y = static_cast<int>(office_json.at(OfficeFields::Y).as_int64())},
        model::Offset{.dx = static_cast<int>(
                          office_json.at(OfficeFields::OFFSET_X).as_int64()),
                      .dy = static_cast<int>(
                          office_json.at(OfficeFields::OFFSET_Y).as_int64())});
}