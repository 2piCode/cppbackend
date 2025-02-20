#include "json_loader.h"

#include <deque>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/json.hpp>
#include <boost/system/detail/error_code.hpp>

#include "json_converter.h"
#include "utils/logger.h"

namespace json_loader {

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
        maps.push_back(map);
    }

    return std::make_shared<app::Game>(maps, default_dog_speed);
}

}  // namespace json_loader
