#pragma once

#include <deque>
#include <memory>
#include <string>

#include "model/model.h"

namespace app {

class GameSession {
   public:
    using DogPointer = model::Dog*;
    using Pointer = std::shared_ptr<GameSession>;

    explicit GameSession(model::Map::ConstPointer map) : map_(map) {}

    const DogPointer AddDog(model::Coordinate spawn_point, std::string name) {
        return &dogs_.emplace_back(dogs_.size(), name, spawn_point);
    }

    model::Map::ConstPointer GetMap() const { return map_; }

    const model::Map::Id GetMapId() const { return map_->GetId(); }
    const std::deque<model::Dog>& GetDogs() const { return dogs_; }

   private:
    model::Map::ConstPointer map_;
    std::deque<model::Dog> dogs_;
};

}  // namespace app
