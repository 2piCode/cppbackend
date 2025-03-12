#pragma once

#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "model/dog.h"
#include "model/item.h"
#include "model/map.h"

namespace app {

class GameSession {
   public:
    using Pointer = std::shared_ptr<GameSession>;
    using LootPositionsVector = std::vector<game::Item>;

    explicit GameSession(const model::Map::Pointer map) : map_(map) {}

    model::Dog::Pointer AddDog(model::Coordinate spawn_point, std::string name,
                               double max_speed) {
        return &dogs_.emplace_back(dogs_.size(), name, max_speed, spawn_point);
    }

    void AddLoot(int type, model::Coordinate pos, int score) {
        if (type > map_->GetNumberOfLootTypes()) {
            throw std::invalid_argument(
                "Loot type incorrected. Max number type: " +
                std::to_string(map_->GetNumberOfLootTypes()) +
                " , but got: " + std::to_string(type));
        }
        loot_positions_.emplace_back(game::Item::Id{item_last_id_++}, type, pos,
                                     score);
    }

    std::optional<game::Item> RemoveLoot(game::Item::Id id) {
        if (auto item_it = std::find_if(
                loot_positions_.begin(), loot_positions_.end(),
                [id](const game::Item& item) { return item.id == id; });
            item_it != loot_positions_.end()) {
            game::Item copy_item{
                .id = item_it->id,
                .type = item_it->type,
                .position = item_it->position,
            };
            loot_positions_.erase(item_it);
            return copy_item;
        }
        return std::nullopt;
    }

    const LootPositionsVector& GetLootPositionsInfo() const noexcept {
        return loot_positions_;
    }

    int GetLootNumber() const noexcept { return loot_positions_.size(); }
    const model::Map::Pointer GetMap() const { return map_; }
    const model::Map::Id GetMapId() const { return map_->GetId(); }
    const std::deque<model::Dog>& GetDogs() const { return dogs_; }

   private:
    std::uint32_t item_last_id_ = 0;

    const model::Map::Pointer map_;
    std::deque<model::Dog> dogs_;
    LootPositionsVector loot_positions_;
};

}  // namespace app
