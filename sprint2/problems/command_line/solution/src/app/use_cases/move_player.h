#pragma once
#include <string>

#include "app/player/players.h"
#include "app/token.h"
#include "base.h"
#include "model/model.h"

enum class MovePlayerErrorReason {
    PlayerNotFound,
};

class MovePlayerError : public UseCaseError {
   public:
    explicit MovePlayerError(std::string code, std::string message,
                             MovePlayerErrorReason reason)
        : UseCaseError(code, message), reason(reason) {}

    MovePlayerErrorReason reason;
};

class MovePlayerUseCase {
   public:
    explicit MovePlayerUseCase(app::Players& players) : players_(players) {}

    void MovePlayer(const app::Token token, const model::Direction direction) {
        auto player = players_.Find(token);

        if (!player) {
            throw MovePlayerError("unknownToken",
                                  "Player token has not been found",
                                  MovePlayerErrorReason::PlayerNotFound);
        }

        player->SetDirection(direction);
    }

   private:
    app::Players& players_;
};
