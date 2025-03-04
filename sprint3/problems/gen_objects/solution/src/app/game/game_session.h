#pragma once

#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "model/dog.h"
#include "model/map.h"

namespace app {

struct Item {
    int type;
    model::Coordinate position;
};

class GameSession {
   public:
    using Pointer = std::shared_ptr<GameSession>;
    using LootPositionsVector = std::vector<Item>;

    explicit GameSession(const model::Map::Pointer map) : map_(map) {}

    model::Dog::Pointer AddDog(model::Coordinate spawn_point, std::string name,
                               double max_speed) {
        return &dogs_.emplace_back(dogs_.size(), name, max_speed, spawn_point);
    }

    void AddLoot(int type, model::Coordinate pos) {
        if (type > map_->GetNumberOfLootTypes()) {
            throw std::invalid_argument(
                "Loot type incorrected. Max number type: " +
                std::to_string(map_->GetNumberOfLootTypes()) +
                " , but got: " + std::to_string(type));
        }
        loot_positions_.emplace_back(type, pos);
    }

    const LootPositionsVector& GetLootPositionsInfo() const noexcept {
        return loot_positions_;
    }

    int GetLootNumber() const noexcept { return loot_positions_.size(); }
    const model::Map::Pointer GetMap() const { return map_; }
    const model::Map::Id GetMapId() const { return map_->GetId(); }
    const std::deque<model::Dog>& GetDogs() const { return dogs_; }

   private:
    const model::Map::Pointer map_;
    std::deque<model::Dog> dogs_;
    LootPositionsVector loot_positions_;
};

}  // namespace app
