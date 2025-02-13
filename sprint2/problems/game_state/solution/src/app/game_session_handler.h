#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "game_session.h"

namespace app {

class GameSessionHandler {
   public:
    using GameSessionPointer = std::shared_ptr<GameSession>;

    GameSessionHandler() = default;
    GameSessionHandler(const GameSessionHandler&) = delete;
    GameSessionHandler& operator=(const GameSessionHandler&) = delete;
    GameSessionHandler(GameSessionHandler&&) = default;

    const GameSessionPointer FindGameSession(
        const model::Map::Id& map_id) const {
        if (auto it = map_id_to_game_session_.find(map_id);
            it != map_id_to_game_session_.end()) {
            // TODO: find the game session with the least amount of players
            return it->second[0];
        }
        return nullptr;
    }

    const GameSessionPointer CreateGameSession(model::Map::ConstPointer map) {
        GameSession new_session(map);
        auto game_session_ptr = game_sessions_.emplace_back(
            std::make_shared<app::GameSession>(map));
        map_id_to_game_session_[map->GetId()].push_back(game_session_ptr);
        return game_session_ptr;
    }

   private:
    using MapIdToGameSession =
        std::unordered_map<model::Map::Id, std::vector<GameSessionPointer>,
                           util::TaggedHasher<model::Map::Id>>;

    std::vector<GameSessionPointer> game_sessions_;
    MapIdToGameSession map_id_to_game_session_;
};

}  // namespace app
