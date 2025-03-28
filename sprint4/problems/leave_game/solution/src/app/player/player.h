#pragma once

#include <chrono>
#include <vector>

#include "app/game/game_session.h"
#include "model/dog.h"
#include "model/model.h"
#include "utils/logger.h"

namespace app {

struct MovementInfo {
    model::Coordinate start_position;
    model::Coordinate end_position;
};

class Player {
   public:
    using Id = model::Dog::Id;
    using Position = model::Coordinate;
    using Velocity = model::Coordinate;
    using Direction = model::Direction;

    using Pointer = Player*;
    using ConstPointer = const Player*;
    using GameSessionPointer = GameSession::Pointer;

    static constexpr double WIDTH = 0.6;

    explicit Player(GameSessionPointer session, model::Dog::Pointer dog)
        : session_(session), dog_(dog) {}

    GameSessionPointer GetSession() const { return session_; }
    Id GetId() const { return dog_->GetId(); }
    model::Dog::ConstPointer GetDog() const { return dog_; }

    void SetDirection(model::Direction direction) {
        dog_->SetDirection(direction);
    }

    MovementInfo Move(std::chrono::milliseconds delta_time);

    void AddItem(model::Item item) { dog_->AddItem(item); }

    std::vector<model::Item> DropAllItems() { return dog_->DropAllItems(); }

    size_t GetItemCount() const { return dog_->GetItemCount(); }

    std::chrono::milliseconds GetTimeInGame() const {
        return GetDog()->GetTimeInGame();
    }

    bool IsAFK(std::chrono::milliseconds retirement_time) const {
        return retirement_time <= GetDog()->GetLastMoveTime();
    }

   private:
    GameSessionPointer session_;
    model::Dog::Pointer dog_;

    MovementInfo MoveToBorder(const model::Coordinate& new_position,
                              const std::vector<model::Road::Pointer>& roads);
};

}  // namespace app
