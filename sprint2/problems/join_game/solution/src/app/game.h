#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <utility>

#include "game_session_handler.h"
#include "model/model.h"
#include "player.h"

namespace app {

class Game {
   public:
    using Map = model::Map;
    using Maps = std::deque<model::Map>;
    using MapPointer = Map*;
    using ConstMapPointer = const Map*;
    using PlayerPointer = Players::PlayerPointer;
    using ConstPlayerPointer = Players::ConstPlayerPointer;
    using TokenPointer = Players::TokenPointer;
    using GameSessionPointer = GameSession::Pointer;

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept { return maps_; }

    ConstMapPointer FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    std::pair<PlayerPointer, TokenPointer> AddPlayer(
        const std::string& username, ConstMapPointer map) {
        if (map == nullptr) {
            return {};
        }

        auto game_session = game_session_handler_.FindGameSession(map->GetId());
        if (!game_session) {
            game_session = CreateGameSession(map);
        }
        auto dog_ptr = game_session->AddDog(username);
        return players_.Add(game_session, dog_ptr);
    }

    ConstPlayerPointer FindPlayer(Token token) const {
        return players_.Find(token);
    }

   private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    GameSessionHandler game_session_handler_;
    Players players_;

    GameSessionPointer FindGameSession(const Map::Id& map_id) const {
        return game_session_handler_.FindGameSession(map_id);
    }

    GameSessionPointer CreateGameSession(const Map* map) {
        return game_session_handler_.CreateGameSession(map);
    }
};

}  // namespace app
