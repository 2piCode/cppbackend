#pragma once

#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "model/dog.h"
#include "model/item.h"
#include "model/map.h"

namespace serialization {
class GameSessionRepr;
}

namespace app {

class GameSession {
   public:
    friend class serialization::GameSessionRepr;

    using Pointer = std::shared_ptr<GameSession>;
    using LootPositionsVector = std::vector<model::Item>;

    explicit GameSession(const model::Map::Pointer map,
                         std::uint32_t last_item_id = 0)
        : map_(map), item_last_id_(last_item_id) {}

    model::Dog::Pointer AddDog(model::Coordinate spawn_point, std::string name,
                               double max_speed) {
        return &dogs_.emplace_back(dogs_.size(), name, max_speed, spawn_point);
    }

    std::optional<model::Dog> RemoveDog(model::Dog::Id id) {
        if (auto dog_it = std::find_if(
                dogs_.begin(), dogs_.end(),
                [id](model::Dog& dog) { return dog.GetId() == id; });
            dog_it != dogs_.end()) {
            model::Dog dog = std::move(*dog_it);
            dogs_.erase(dog_it);
            return dog;
        }
        return std::nullopt;
    }

    void AddLoot(int type, model::Coordinate pos, int score) {
        if (type > map_->GetNumberOfLootTypes()) {
            throw std::invalid_argument(
                "Loot type incorrected. Max number type: " +
                std::to_string(map_->GetNumberOfLootTypes()) +
                " , but got: " + std::to_string(type));
        }
        loot_positions_.emplace_back(model::Item::Id{item_last_id_++}, type,
                                     pos, score);
    }

    std::optional<model::Item> RemoveLoot(model::Item::Id id) {
        if (auto item_it = std::find_if(
                loot_positions_.begin(), loot_positions_.end(),
                [id](const model::Item& item) { return item.id == id; });
            item_it != loot_positions_.end()) {
            model::Item copy_item{
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
    std::uint32_t GetLastItemId() const { return item_last_id_; }

   private:
    std::uint32_t item_last_id_ = 0;

    const model::Map::Pointer map_;
    std::deque<model::Dog> dogs_;
    LootPositionsVector loot_positions_;
};

}  // namespace app
