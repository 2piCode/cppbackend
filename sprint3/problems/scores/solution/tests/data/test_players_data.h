#pragma once

#include <chrono>
#include <memory>

#include "app/game/game_session.h"
#include "app/player/players.h"
#include "app/use_cases/get_game_state_use_case.h"
#include "model/dog.h"

namespace test_players {
using namespace std::string_literals;

class EmptyPlayers : public app::PlayersCollection {
   public:
    app::Player::Pointer Find(app::Token token) const override {
        return nullptr;
    }

    /*void Tick(std::chrono::milliseconds) override {}*/
};

class OnePlayerPlayers : public app::PlayersCollection {
   public:
    ~OnePlayerPlayers() { delete player_; }
    app::Player::Pointer Find(app::Token token) const override {
        return player_;
    }

    PlayerGameState GetPlayerGameState() const {
        return PlayerGameState{.id = player_->GetId(),
                               .position = player_->GetDog()->GetPosition(),
                               .velocity = player_->GetDog()->GetVelocity(),
                               .direction = player_->GetDog()->GetDirection()};
    }

    app::GameSession::Pointer GetSession() const noexcept { return session_; }

   private:
    model::Map::Pointer map_ = std::make_shared<model::Map>(
        model::Map::Id{"map"}, "map", model::Map::Roads{},
        model::Map::Buildings{}, model::Map::Offices{}, 1.0, 10);
    app::GameSession::Pointer session_ =
        std::make_shared<app::GameSession>(map_);
    model::Dog::Pointer dog_ = session_->AddDog(model::Coordinate{0.0, 0.0},
                                                "dog"s, *map_->GetMaxSpeed());
    app::Player::Pointer player_ = new app::Player(session_, dog_);
};
}  // namespace test_players
