#pragma once

#include <deque>
#include <memory>
#include <string>

#include "model/dog.h"
#include "model/map.h"

namespace app {

class GameSession {
   public:
    using Pointer = std::shared_ptr<GameSession>;

    explicit GameSession(const model::Map::Pointer map) : map_(map) {}

    model::Dog::Pointer AddDog(model::Coordinate spawn_point, std::string name,
                               double max_speed) {
        return &dogs_.emplace_back(dogs_.size(), name, max_speed, spawn_point);
    }

    const model::Map::Pointer GetMap() const { return map_; }

    const model::Map::Id GetMapId() const { return map_->GetId(); }
    const std::deque<model::Dog>& GetDogs() const { return dogs_; }

   private:
    const model::Map::Pointer map_;
    std::deque<model::Dog> dogs_;
};

}  // namespace app
