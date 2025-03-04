#pragma once

#include <string>
#include <utility>

#include <boost/json/array.hpp>
#include <boost/json/object.hpp>

#include "app/game/game.h"
#include "app/use_cases/base.h"
#include "loot_handler.h"

enum class GetMapErrorReason { MapNotFound, LootTypesNotFound };

struct GetMapError : public UseCaseError {
   public:
    GetMapError(std::string code, std::string message, GetMapErrorReason reason)
        : UseCaseError(code, message), reason(reason) {}

    GetMapErrorReason reason;
};

struct GetMapResult {
    const model::Map::Pointer map_ptr;
    boost::json::array loot_types;
};

class GetMapUseCase {
   public:
    explicit GetMapUseCase(app::Game::Pointer game,
                           LootHandler::Pointer&& handler)
        : game_(game), loot_handler_(std::move(handler)) {}

    GetMapResult GetMap(const model::Map::Id& map_id) const {
        auto map = game_->FindMap(map_id);
        if (map == nullptr) {
            throw GetMapError("mapNotFound", "Map not found",
                              GetMapErrorReason::MapNotFound);
        }

        if (auto loot_types = loot_handler_->FindLootType(map_id);
            loot_types.has_value()) {
            return {.map_ptr = map, .loot_types = *loot_types};
        }

        throw GetMapError("mapNotFound", "Map not found",
                          GetMapErrorReason::LootTypesNotFound);
    }

   private:
    app::Game::Pointer game_;
    LootHandler::Pointer loot_handler_;
};
