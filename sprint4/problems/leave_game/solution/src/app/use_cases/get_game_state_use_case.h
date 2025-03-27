#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/json/object.hpp>

#include "app/player/players.h"
#include "app/token.h"
#include "app/use_cases/base.h"
#include "model/item.h"
#include "utils/logger.h"

enum class GetGameStateErrorReason { UnknownToken };

struct GetGameStateError : public UseCaseError {
    GetGameStateError(std::string code, std::string message,
                      GetGameStateErrorReason reason)
        : UseCaseError(code, message), reason(reason) {}

    GetGameStateErrorReason reason;
};

struct PlayerGameState {
    app::Player::Id id;
    app::Player::Position position;
    app::Player::Velocity velocity;
    app::Player::Direction direction;
    std::vector<model::Item> items;
    int score;
};

struct GameState {
    std::vector<PlayerGameState> player_coord_infos;
    std::vector<model::Item> lost_objects;
};

class GetGameStateUseCase {
   public:
    explicit GetGameStateUseCase(
        std::shared_ptr<app::PlayersCollection> players)
        : players_(players) {}

    GameState GetGameState(const app::Token& token) const {
        auto player = players_->Find(token);
        if (!player) {
            throw GetGameStateError{"unknownToken",
                                    "Player token has not been found",
                                    GetGameStateErrorReason::UnknownToken};
        }

        GameState result;
        auto session_ptr = player->GetSession();
        const auto& dogs = session_ptr->GetDogs();
        result.player_coord_infos.reserve(dogs.size());
        for (const auto& dog : dogs) {
            result.player_coord_infos.push_back(
                {dog.GetId(), dog.GetPosition(), dog.GetVelocity(),
                 dog.GetDirection(), dog.GetItems(), dog.GetScore()});
        }

        const auto& lost_objects = session_ptr->GetLootPositionsInfo();
        result.lost_objects.reserve(lost_objects.size());
        for (const auto& object : lost_objects) {
            result.lost_objects.emplace_back(object.id, object.type,
                                             object.position);
        }

        return result;
    }

   private:
    const std::shared_ptr<app::PlayersCollection> players_;
};
