#pragma once

#include <random>
#include <string>

#include "app/game/game.h"
#include "app/player/players.h"
#include "app/token.h"
#include "app/use_cases/base.h"
#include "model/model.h"
#include "utils/logger.h"

enum class JoinGameErrorReason { InvalidName, InvalidMap };

struct JoinGameResult {
    app::Token token;
    app::Player::Id player_id;
};

struct JoinGameError : public UseCaseError {
    JoinGameError(std::string code, std::string message,
                  JoinGameErrorReason reason)
        : UseCaseError(code, message), reason(reason) {}
    JoinGameErrorReason reason;
};

namespace app {
class JoinGameUseCase {
   public:
    JoinGameUseCase(Game::Pointer game, Players& players,
                    const bool is_random_spawn_point)
        : game_(game),
          players_(players),
          is_random_spawn_point_(is_random_spawn_point) {}

    JoinGameResult Join(const model::Map::Id& map_id,
                        const std::string& user_name) {
        if (user_name.length() == 0) {
            throw JoinGameError("invalidArgument", "Invalid name",
                                JoinGameErrorReason::InvalidName);
        }

        if (auto session = game_->FindGameSession(map_id); session) {
            auto map = session->GetMap();
            double dog_speed = game_->GetDefaultDogSpeed();
            if (map->GetMaxSpeed().has_value()) {
                dog_speed = map->GetMaxSpeed().value();
            }
            auto spawn_point = GenerateSpawnPoint(session->GetMap());
            auto player_token = players_.Add(
                session, session->AddDog(spawn_point, user_name, dog_speed));
            return {player_token.second, player_token.first};
        }

        throw JoinGameError("mapNotFound", "Invalid map",
                            JoinGameErrorReason::InvalidMap);
    }

   private:
    Game::Pointer game_;
    Players& players_;
    bool is_random_spawn_point_;

    std::random_device random_device_;
    std::mt19937_64 generator1_{random_device_()};

    model::Coordinate GenerateSpawnPoint(const model::Map::Pointer map) {
        auto roads = map->GetRoads();

        if (!is_random_spawn_point_)
            return model::Coordinate{
                .x = static_cast<double>(roads[0]->GetStart().x),
                .y = static_cast<double>(roads[0]->GetStart().y)};

        std::uniform_int_distribution<std::uint32_t> distribution_roads(
            0, roads.size() - 1);
        auto road = roads[distribution_roads(generator1_)];

        std::uniform_int_distribution<std::int32_t> distributionX(
            road->GetStart().x, road->GetEnd().x);
        std::uniform_int_distribution<std::int32_t> distributionY(
            road->GetStart().y, road->GetEnd().y);

        return {static_cast<double>(distributionX(generator1_)),
                static_cast<double>(distributionY(generator1_))};
    }
};
}  // namespace app
