#pragma once

#include <memory>

#include "app/game/game.h"
#include "model/map.h"
#include "model/model.h"

class EmptyGame : public app::Game {
   public:
    using app::Game::Game;
    const Map::Pointer FindMap(const Map::Id& id) const noexcept {
        return nullptr;
    }
};

class GameWithOneMap : public app::Game {
   public:
    using app::Game::Game;

    const Map::Pointer FindMap(const Map::Id& id) const noexcept {
        return map_;
    }
    GameSessionPointer FindGameSession(const Map::Id& map_id) override {
        return game_session_;
    }
    const Maps& GetMaps() const noexcept override { return maps_; }

    const Map::Pointer GetCorrectMap() const noexcept { return map_; }
    const app::GameSession::Pointer GetSession() const noexcept {
        return game_session_;
    }

   private:
    Maps maps_;
    model::Map::Roads roads_;
    model::Road::Pointer road_ =
        roads_.emplace_back(std::make_shared<model::Road>(
            model::Road::HORIZONTAL, model::Point{0, 0}, 1));

    model::Map::Pointer map_ = maps_.emplace_back(std::make_shared<model::Map>(
        model::Map::Id{"id"}, "", roads_, model::Map::Buildings{},
        model::Map::Offices{}, 1.0, 10));
    app::GameSession::Pointer game_session_ =
        std::make_shared<app::GameSession>(map_);
    model::Dog::Pointer dog_ =
        game_session_->AddDog(model::Coordinate{0.0, 0.0}, "dog_name", 10.0);
};
