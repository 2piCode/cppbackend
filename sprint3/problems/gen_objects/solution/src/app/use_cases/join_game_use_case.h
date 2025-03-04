#pragma once

#include <memory>
#include <string>

#include "app/game/game.h"
#include "app/player/players.h"
#include "app/spawn_point_generator.h"
#include "app/token.h"
#include "app/use_cases/base.h"
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
    JoinGameUseCase(Game::Pointer game,
                    std::shared_ptr<app::PlayersCollection> players,
                    const bool is_random_spawn_point)
        : game_(game),
          players_(players),
          spawn_point_generator_(is_random_spawn_point) {}

    JoinGameResult Join(const model::Map::Id& map_id,
                        const std::string& user_name) {
        if (user_name.length() == 0) {
            throw JoinGameError("invalidArgument", "Invalid name",
                                JoinGameErrorReason::InvalidName);
        }

        auto session = game_->FindGameSession(map_id);
        if (!session) {
            session = game_->CreateGameSession(map_id);
        }

        if (session) {
            auto map = session->GetMap();
            double dog_speed = game_->GetDefaultDogSpeed();
            if (map->GetMaxSpeed().has_value()) {
                dog_speed = map->GetMaxSpeed().value();
            }
            auto spawn_point =
                spawn_point_generator_.Generate(session->GetMap());
            auto player_token = players_->Add(
                session, session->AddDog(spawn_point, user_name, dog_speed));
            return {player_token.second, player_token.first};
        }

        throw JoinGameError("mapNotFound", "Invalid map",
                            JoinGameErrorReason::InvalidMap);
    }

   private:
    Game::Pointer game_;
    std::shared_ptr<app::PlayersCollection> players_;
    SpawnPointGenerator spawn_point_generator_;
};
}  // namespace app
