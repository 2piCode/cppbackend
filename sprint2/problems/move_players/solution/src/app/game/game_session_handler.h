#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "app/game/game_session.h"

namespace app {

class GameSessionHandler {
   public:
    GameSessionHandler() = default;
    GameSessionHandler(const GameSessionHandler&) = delete;
    GameSessionHandler& operator=(const GameSessionHandler&) = delete;
    GameSessionHandler(GameSessionHandler&&) = default;

    const GameSession::Pointer FindGameSession(
        const model::Map::Id& map_id) const;

    const GameSession::Pointer CreateGameSession(model::Map::ConstPointer map);

   private:
    using MapIdToGameSession =
        std::unordered_map<model::Map::Id, std::vector<GameSession::Pointer>,
                           util::TaggedHasher<model::Map::Id>>;

    std::vector<GameSession::Pointer> game_sessions_;
    MapIdToGameSession map_id_to_game_session_;
};

}  // namespace app
