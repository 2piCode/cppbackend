#pragma once

#include <boost/json.hpp>

#include "boost/json/object.hpp"
#include "model.h"

namespace json_converter {
namespace json = boost::json;

json::object MapToJson(const model::Map& map);

json::object FullMapToJson(const model::Map& map);

json::array RoadsToJson(const std::vector<model::Road>& roads);

json::object RoadToJson(const model::Road& road);

json::array BuildingsToJson(const std::vector<model::Building>& buildings);

boost::json::object BuildingToJson(const model::Building& building);

boost::json::array OfficesToJson(const std::vector<model::Office>& offices);

boost::json::object OfficeToJson(const model::Office& office);

model::Map JsonToMap(const json::object map_json);

std::vector<model::Road> JsonToRoads(const json::array roads_json);

model::Road JsonToRoad(const json::object road_json);

std::vector<model::Building> JsonToBuildings(const json::array buildings_json);

model::Building JsonToBuilding(const json::object building_json);

std::vector<model::Office> JsonToOffices(const json::array offices_json);

model::Office JsonToOffice(const json::object office);

}  // namespace json_converter
