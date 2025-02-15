#pragma once

#include <string>

#include "app/game/game.h"
#include "app/use_cases/base.h"

enum class GetMapErrorReason { MapNotFound };

struct GetMapError : public UseCaseError {
   public:
    GetMapError(std::string code, std::string message, GetMapErrorReason reason)
        : UseCaseError(code, message), reason(reason) {}

    GetMapErrorReason reason;
};

class GetMapUseCase {
   public:
    explicit GetMapUseCase(app::Game::Pointer game) : game_(game) {}

    model::Map::ConstPointer GetMap(const model::Map::Id& map_id) const {
        auto map = game_->FindMap(map_id);
        if (map == nullptr) {
            throw GetMapError("mapNotFound", "Map not found",
                              GetMapErrorReason::MapNotFound);
        }
        return map;
    }

   private:
    app::Game::Pointer game_;
};
