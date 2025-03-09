#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "model/model.h"
#include "model/roads_handler.h"

namespace model {

class Map {
   public:
    using Id = util::Tagged<std::string, Map>;
    using Pointer = std::shared_ptr<Map>;
    using Roads = RoadsHandler::Roads;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name, Roads roads, Buildings buildings,
        Offices offices, std::optional<double> dog_speed,
        int number_loot_types) noexcept
        : id_(std::move(id)),
          name_(std::move(name)),
          roads_handler_(std::move(roads)),
          buildings_(std::move(buildings)),
          offices_(std::move(offices)),
          dog_speed_(dog_speed),
          number_loot_types_(number_loot_types) {}

    const Id& GetId() const noexcept { return id_; }

    const std::string& GetName() const noexcept { return name_; }

    const Buildings& GetBuildings() const noexcept { return buildings_; }

    const Roads& GetRoads() const noexcept { return roads_handler_.GetRoads(); }

    const Offices& GetOffices() const noexcept { return offices_; }

    std::optional<double> GetMaxSpeed() const noexcept { return dog_speed_; }

    int GetNumberOfLootTypes() const noexcept { return number_loot_types_; }

    std::vector<Road::Pointer> FindRoads(Coordinate pos) const {
        return roads_handler_.FindRoads(pos);
    }

   private:
    const Id id_;
    const std::string name_;
    const RoadsHandler roads_handler_;
    const Buildings buildings_;
    const Offices offices_;
    const std::optional<double> dog_speed_;
    const int number_loot_types_;
};

}  // namespace model
