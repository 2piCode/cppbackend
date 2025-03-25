#pragma once

#include <vector>

#include <boost/json.hpp>
#include <boost/json/object.hpp>

#include "app/use_cases/get_game_records_use_case.h"
#include "app/use_cases/get_game_state_use_case.h"
#include "app/use_cases/list_player_use_case.h"
#include "model/item.h"
#include "model/model.h"

namespace json_converter {
namespace json = boost::json;

json::object MapToJson(const model::Map& map);

json::object FullMapToJson(const model::Map& map);

json::array RoadsToJson(const model::Map::Roads& roads);

json::object RoadToJson(const model::Road& road);

json::array BuildingsToJson(const std::vector<model::Building>& buildings);

boost::json::object BuildingToJson(const model::Building& building);

boost::json::array OfficesToJson(const std::vector<model::Office>& offices);

boost::json::object OfficeToJson(const model::Office& office);

boost::json::object PlayerInfoToJson(const PlayerInfo& player_info);

boost::json::object ItemToJson(model::Item item_info);

boost::json::object GameStateToJson(const GameState& game_state);

boost::json::object PlayerGameStateToJson(
    const PlayerGameState& player_game_state);

boost::json::object GameRecordToJson(const GameRecord& game_record);

model::Map::Pointer JsonToMap(const json::object map_json);

model::Map::Roads JsonToRoads(const json::array roads_json);

model::Road::Pointer JsonToRoad(const json::object road_json);

std::vector<model::Building> JsonToBuildings(const json::array buildings_json);

model::Building JsonToBuilding(const json::object building_json);

std::vector<model::Office> JsonToOffices(const json::array offices_json);

model::Office JsonToOffice(const json::object office_json);

}  // namespace json_converter
