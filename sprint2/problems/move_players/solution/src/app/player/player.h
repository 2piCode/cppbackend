#pragma once

#include "app/game/game_session.h"
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

    explicit Player(GameSessionPointer session, model::Dog* dog)
        : session_(session), dog_(dog) {}

    GameSessionPointer GetSession() const { return session_; }
    Id GetId() const { return dog_->GetId(); }
    model::Dog* GetDog() const { return dog_; }

   private:
    GameSessionPointer session_;
    model::Dog* dog_;
};

}  // namespace app
