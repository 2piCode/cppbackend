#pragma once

#include <string>
#include <vector>

#include "app/player/players.h"
#include "app/token.h"
#include "app/use_cases/base.h"

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
};

struct GameState {
    std::vector<PlayerGameState> player_coord_infos;
};

class GetGameStateUseCase {
   public:
    explicit GetGameStateUseCase(const app::Players& players)
        : players_(players) {}

    GameState GetGameState(const app::Token& token) const {
        auto player = players_.Find(token);
        if (!player) {
            throw GetGameStateError{"unknownToken",
                                    "Player token has not been found",
                                    GetGameStateErrorReason::UnknownToken};
        }

        GameState result;
        const auto& dogs = player->GetSession()->GetDogs();
        result.player_coord_infos.reserve(dogs.size());
        for (const auto& dog : dogs) {
            result.player_coord_infos.push_back({dog.GetId(), dog.GetPosition(),
                                                 dog.GetVelocity(),
                                                 dog.GetDirection()});
        }

        return result;
    }

   private:
    const app::Players& players_;
};
