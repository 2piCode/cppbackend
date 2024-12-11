#include "json_converter.h"

#include "model.h"

boost::json::object json_converter::MapToJson(const model::Map& map) {
    boost::json::object main;

    main["id"] = *map.GetId();
    main["name"] = map.GetName();

    main["roads"] = RoadsToJson(map.GetRoads());
    main["buildings"] = BuildingsToJson(map.GetBuildings());
    main["offices"] = OfficesToJson(map.GetOffices());

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

    main["x0"] = road.GetStart().x;
    main["y0"] = road.GetStart().y;
    if (road.GetStart().x != road.GetEnd().x) {
        main["x1"] = road.GetEnd().x;
    } else {
        main["y1"] = road.GetEnd().y;
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

    main["x"] = building.GetBounds().position.x;
    main["y"] = building.GetBounds().position.y;
    main["w"] = building.GetBounds().size.width;
    main["h"] = building.GetBounds().size.height;

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

    main["id"] = *office.GetId();
    main["x"] = office.GetPosition().x;
    main["y"] = office.GetPosition().y;
    main["offsetX"] = office.GetOffset().dx;
    main["offsetY"] = office.GetOffset().dy;

    return main;
}

model::Map json_converter::JsonToMap(json::object map_json) {
    model::Map map(model::Map::Id{map_json.at("id").as_string().c_str()},
                   map_json.at("name").as_string().c_str());

    return map;
}

std::vector<model::Road> json_converter::JsonToRoads(
    boost::json::array roads_json) {
    std::vector<model::Road> roads;

    for (const auto& road_json : roads_json) {
        roads.push_back(JsonToRoad(road_json.as_object()));
    }

    return roads;
}

model::Road json_converter::JsonToRoad(json::object road) {
    model::Point start{.x = static_cast<int>(road.at("x0").as_int64()),
                       .y = static_cast<int>(road.at("y0").as_int64())};

    auto x_cord = road.try_at("x1");
    if (x_cord) {
        return model::Road(model::Road::HORIZONTAL, start,
                           static_cast<int>(x_cord->as_int64()));
    }
    return model::Road(model::Road::VERTICAL, start,
                       static_cast<int>(road.at("y1").as_int64()));
}

std::vector<model::Building> json_converter::JsonToBuildings(
    boost::json::array buildings_json) {
    std::vector<model::Building> buildings;

    for (const auto& building_json : buildings_json) {
        buildings.push_back(JsonToBuilding(building_json.as_object()));
    }

    return buildings;
}

model::Building json_converter::JsonToBuilding(boost::json::object building) {
    return model::Building(model::Rectangle{
        .position =
            model::Point{.x = static_cast<int>(building.at("x").as_int64()),
                         .y = static_cast<int>(building.at("y").as_int64())},
        .size = model::Size{
            .width = static_cast<int>(building.at("w").as_int64()),
            .height = static_cast<int>(building.at("h").as_int64())}});
}

std::vector<model::Office> json_converter::JsonToOffices(
    boost::json::array offices_json) {
    std::vector<model::Office> offices;
    for (const auto& office_json : offices_json) {
        offices.push_back(JsonToOffice(office_json.as_object()));
    }
    return offices;
}

model::Office json_converter::JsonToOffice(boost::json::object office) {
    return model::Office(
        model::Office::Id{office.at("id").as_string().c_str()},
        model::Point{.x = static_cast<int>(office.at("x").as_int64()),
                     .y = static_cast<int>(office.at("y").as_int64())},
        model::Offset{.dx = static_cast<int>(office.at("offsetX").as_int64()),
                      .dy = static_cast<int>(office.at("offsetY").as_int64())});
}
