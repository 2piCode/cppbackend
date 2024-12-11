#pragma once

#include <boost/json.hpp>

#include "boost/json/object.hpp"
#include "model.h"

namespace json_converter {
namespace json = boost::json;

json::object MapToJson(const model::Map& map);

json::array RoadsToJson(const std::vector<model::Road>& roads);

json::object RoadToJson(const model::Road& road);

json::array BuildingsToJson(const std::vector<model::Building>& buildings);

boost::json::object BuildingToJson(const model::Building& building);

boost::json::array OfficesToJson(const std::vector<model::Office>& offices);

boost::json::object OfficeToJson(const model::Office& office);

model::Map JsonToMap(json::object map);

std::vector<model::Road> JsonToRoads(json::array roads);

model::Road JsonToRoad(json::object road);

std::vector<model::Building> JsonToBuildings(json::array buildings);

model::Building JsonToBuilding(json::object building);

std::vector<model::Office> JsonToOffices(json::array offices);

model::Office JsonToOffice(json::object office);

}  // namespace json_converter
