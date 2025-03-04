#pragma once

#include <chrono>
#include <vector>

#include "app/game/game_session.h"
#include "model/dog.h"
#include "model/model.h"

namespace app {

class Player {
   public:
    using Id = model::Dog::Id;
    using Position = model::Coordinate;
    using Velocity = model::Coordinate;
    using Direction = model::Direction;

    using Pointer = Player*;
    using ConstPointer = const Player*;
    using GameSessionPointer = GameSession::Pointer;

    explicit Player(GameSessionPointer session, model::Dog::Pointer dog)
        : session_(session), dog_(dog) {}

    GameSessionPointer GetSession() const { return session_; }
    Id GetId() const { return dog_->GetId(); }
    model::Dog::ConstPointer GetDog() const { return dog_; }

    void SetDirection(model::Direction direction) {
        dog_->SetDirection(direction);
    }
    void Tick(std::chrono::milliseconds delta_time) { Move(delta_time); }

   private:
    GameSessionPointer session_;
    model::Dog::Pointer dog_;

    void Move(std::chrono::milliseconds delta_time);

    void MoveToBorder(const model::Coordinate& new_position,
                      const std::vector<model::Road::Pointer>& roads);
};

}  // namespace app
