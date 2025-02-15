#include "json_loader.h"

#include <deque>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
//
#include <boost/json.hpp>
#include <boost/system/detail/error_code.hpp>

#include "json_converter.h"

namespace json_loader {

void addRoads(model::Map& map, const boost::json::object& map_json) {
    for (const auto& road : map_json.at("roads").as_array()) {
        map.AddRoad(json_converter::JsonToRoad(road.as_object()));
    }
}

void addBuildings(model::Map& map, const boost::json::object& map_json) {
    for (const auto& building : map_json.at("buildings").as_array()) {
        map.AddBuilding(json_converter::JsonToBuilding(building.as_object()));
    }
}

void addOffices(model::Map& map, const boost::json::object& map_json) {
    for (const auto& office : map_json.at("offices").as_array()) {
        map.AddOffice(json_converter::JsonToOffice(office.as_object()));
    }
}

void addInfrastracture(model::Map& map, const boost::json::object& map_json) {
    addRoads(map, map_json);
    addBuildings(map, map_json);
    addOffices(map, map_json);
}

app::Game::Pointer LoadGame(const std::filesystem::path& json_path) {
    std::ifstream json_input_stream(json_path);
    boost::system::error_code ec;
    if (!json_input_stream.is_open()) {
        throw std::runtime_error("Incorrect input file");
    }

    std::stringstream buffer;
    buffer << json_input_stream.rdbuf();
    std::string jsonString = buffer.str();

    auto value = boost::json::parse(jsonString, ec);

    if (ec) {
        throw std::runtime_error("Incorrect input file");
    }

    auto value_json_object = value.as_object();

    double default_dog_speed = 1.0;
    if (auto default_dog_speed_it = value_json_object.find("defaultDogSpeed");
        default_dog_speed_it != value_json_object.end()) {
        default_dog_speed = default_dog_speed_it->value().as_double();
    }

    auto maps_json = value_json_object["maps"].as_array();
    std::deque<model::Map> maps;
    for (const auto& map_json : maps_json) {
        const auto map_json_object = map_json.as_object();
        model::Map map = json_converter::JsonToMap(map_json_object);
        addInfrastracture(map, map_json_object);
        maps.push_back(map);
    }

    return std::make_shared<app::Game>(maps, default_dog_speed);
}

}  // namespace json_loader
