#pragma once

#include <string>

#include "app/game.h"
#include "app/player.h"
#include "base.h"
#include "model/model.h"

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
    JoinGameUseCase(Game::Pointer game, Players& players)
        : game_(game), players_(players) {}

    JoinGameResult Join(const model::Map::Id& map_id,
                        const std::string& user_name) {
        if (user_name.length() == 0) {
            throw JoinGameError("invalidArgument", "Invalid name",
                                JoinGameErrorReason::InvalidName);
        }

        if (auto session = game_->FindGameSession(map_id)) {
            auto spawn_point = GenerateSpawnPoint(session->GetMap());
            auto player_token =
                players_.Add(session, session->AddDog(spawn_point, user_name));
            return {player_token.second, player_token.first};
        }

        throw JoinGameError("mapNotFound", "Invalid map",
                            JoinGameErrorReason::InvalidMap);
    }

   private:
    Game::Pointer game_;
    Players& players_;

    model::Coordinate GenerateSpawnPoint(model::Map::ConstPointer map) {
        return {0, 0};
    }
};
}  // namespace app
