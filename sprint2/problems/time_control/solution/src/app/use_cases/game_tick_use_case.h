#pragma once

#include <string>

#include "app/player/players.h"
#include "base.h"

enum class GameTickErrorReason { InvalidDeltaTime };

struct GameTickError : public UseCaseError {
    GameTickError(std::string code, std::string message,
                  GameTickErrorReason reason)
        : UseCaseError(code, message), reason_(reason) {}
    GameTickErrorReason reason_;
};

class GameTickUseCase {
   public:
    explicit GameTickUseCase(app::Players& players) : players_(players) {}

    void Tick(int delta_time) {
        if (delta_time <= 0) {
            throw GameTickError("invalidArgument", "Invalid delta time",
                                GameTickErrorReason::InvalidDeltaTime);
        }
        players_.Tick(delta_time);
    }

   private:
    app::Players& players_;
};
