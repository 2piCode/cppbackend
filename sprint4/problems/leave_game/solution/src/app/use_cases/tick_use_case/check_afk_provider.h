#pragma once

#include <memory>

#include <boost/log/trivial.hpp>

#include "app/game/game.h"
#include "app/player/players.h"
#include "postgres/unit_of_work.h"

class CheckAFKProvider {
   public:
    CheckAFKProvider(app::Game::Pointer game,
                     std::shared_ptr<app::PlayersCollection> players,
                     std::shared_ptr<postgres::UnitOfWorkFactory> factory)
        : game_(game), players_(players), factory_(factory) {}

    void CheckAFKPlayers() {
        for (const auto& player : players_->GetPlayers()) {
            if (player.IsAFK(game_->GetDogRetirementTime())) {
                RemovePlayer(player);
            }
        }
    }

   private:
    app::Game::Pointer game_;
    std::shared_ptr<app::PlayersCollection> players_;
    std::shared_ptr<postgres::UnitOfWorkFactory> factory_;

    void RemovePlayer(const app::Player& player) {
        auto player_id = player.GetId();
        auto dog = player.GetDog();
        players_->Remove(player_id);

        WritePlayerInfo(*dog);
    }

    void WritePlayerInfo(const model::Dog& dog) {
        postgres::PlayerInfo info{
            .name = std::string(dog.GetName()),
            .score = dog.GetScore(),
            .time_in_game =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                    dog.GetTimeInGame())
                    .count()};
        auto unit_of_work = factory_->CreateUnitOfWork();
        unit_of_work->GetPlayers().Write(info);
        unit_of_work->Commit();
    }
};
