#include "json_loader.h"

#include <chrono>
#include <deque>
#include <fstream>
#include <memory>
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/json.hpp>
#include <boost/system/detail/error_code.hpp>

#include "app/game/game.h"
#include "app/game/game_session_handler.h"
#include "json_converter.h"
#include "loots/loot_generator.h"

namespace json_loader {

boost::json::object ParseJson(const std::filesystem::path& json_path) {
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

    return value.as_object();
}

app::Game::Pointer LoadGame(const std::filesystem::path& json_path) {
    auto json_object = ParseJson(json_path);
    double default_dog_speed = 1.0;
    if (auto default_dog_speed_it = json_object.find("defaultDogSpeed");
        default_dog_speed_it != json_object.end()) {
        default_dog_speed = default_dog_speed_it->value().as_double();
    }

    std::chrono::milliseconds dog_retirement_time =
        app::detail::DEFAULT_DOG_RETIREMENT_TIME;
    if (auto dog_retirement_time_it = json_object.find("dogRetirementTime");
        dog_retirement_time_it != json_object.end()) {
        dog_retirement_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double>(
                    dog_retirement_time_it->value().as_double()));
    }

    auto maps_json = json_object["maps"].as_array();
    std::deque<model::Map::Pointer> maps;
    for (const auto& map_json : maps_json) {
        const auto map_json_object = map_json.as_object();
        model::Map::Pointer map = json_converter::JsonToMap(map_json_object);
        maps.push_back(map);
    }

    return std::make_shared<app::Game>(
        maps, default_dog_speed, std::make_shared<app::GameSessionHandler>(),
        dog_retirement_time);
}

loot_gen::LootGenerator::Pointer LoadLootGenerator(
    const std::filesystem::path& json_path) {
    auto json_object = ParseJson(json_path);
    auto lootGeneratorConfigObject =
        json_object["lootGeneratorConfig"].as_object();
    double period = lootGeneratorConfigObject["period"].as_double();
    auto time_interval =
        loot_gen::LootGenerator::TimeInterval(static_cast<int>(period));

    return std::make_shared<loot_gen::LootGenerator>(
        time_interval, lootGeneratorConfigObject["probability"].as_double(),
        loot_gen::LootGenerator::TimeInterval(0));
}

LootHandler::Pointer LoadLootHandler(const std::filesystem::path& json_path) {
    auto json_object = ParseJson(json_path);

    auto maps_json = json_object["maps"].as_array();
    LootHandler::LootTypeByMap loot_types_by_map;
    LootHandler::LootTypeScoreByMap loot_type_score_by_map;
    for (const auto& map_json : maps_json) {
        auto map_json_object = map_json.as_object();
        model::Map::Id map_id =
            json_converter::JsonToMap(map_json_object)->GetId();
        auto loot_types_array = map_json_object["lootTypes"].as_array();
        loot_types_by_map.emplace(map_id, loot_types_array);

        for (const auto& loot_type_el : loot_types_array) {
            auto el_obj = loot_type_el.as_object();
            loot_type_score_by_map[map_id].push_back(
                el_obj["value"].as_int64());
        }
    }

    return std::make_shared<LootHandler>(std::move(loot_types_by_map),
                                         std::move(loot_type_score_by_map));
}

LootNumberMapHandler::Pointer LoadNumberMapHandler(
    const std::filesystem::path& json_path) {
    auto json_object = ParseJson(json_path);

    int base_loot_max_number = 3;
    if (auto it = json_object.find("defaultBagCapacity");
        it != json_object.end()) {
        base_loot_max_number = json_object["defaultBagCapacity"].as_int64();
    }

    LootNumberMapHandler::LootNumberByMap max_loot_number_by_map;

    auto maps_json = json_object["maps"].as_array();
    for (const auto& map_json : maps_json) {
        auto map_json_object = map_json.as_object();
        model::Map::Id map_id =
            json_converter::JsonToMap(map_json_object)->GetId();
        if (auto it = map_json_object.find("bagCapacity");
            it != map_json_object.end()) {
            max_loot_number_by_map.emplace(map_id, it->value().as_int64());
        }
    }

    return std::make_shared<LootNumberMapHandler>(
        std::move(max_loot_number_by_map), base_loot_max_number);
}

}  // namespace json_loader
