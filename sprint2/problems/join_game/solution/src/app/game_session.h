#pragma once

#include <string>
#include <vector>

#include "model/model.h"

namespace app {

class GameSession {
   public:
    using DogPointer = model::Dog*;
    using Pointer = std::shared_ptr<GameSession>;

    explicit GameSession(const model::Map* map) : map_(map) {}

    const DogPointer AddDog(const std::string& name) {
        return &dogs_.emplace_back(dogs_.size(), name);
    }

    const model::Map::Id GetMapId() const { return map_->GetId(); }
    const std::vector<model::Dog>& GetDogs() const { return dogs_; }

   private:
    const model::Map* map_;
    std::vector<model::Dog> dogs_;
};

}  // namespace app
