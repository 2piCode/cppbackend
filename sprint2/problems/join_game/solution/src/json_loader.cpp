#include "json_loader.h"

#include <boost/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "boost/system/detail/error_code.hpp"
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

app::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    app::Game game;

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

    auto maps = value.as_object()["maps"].as_array();

    for (const auto& map_json : maps) {
        const auto map_json_object = map_json.as_object();
        model::Map map = json_converter::JsonToMap(map_json_object);
        addInfrastracture(map, map_json_object);

        game.AddMap(map);
    }

    return game;
}

}  // namespace json_loader
