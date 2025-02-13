#pragma once

#include <deque>
#include <memory>
#include <unordered_map>
#include <utility>

#include "game_session_handler.h"
#include "model/model.h"

namespace app {

class Game {
   public:
    using Pointer = std::shared_ptr<Game>;

    using Map = model::Map;
    using Maps = std::deque<model::Map>;
    using GameSessionPointer = GameSession::Pointer;

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept { return maps_; }

    Map::ConstPointer FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return std::make_shared<Map>(maps_.at(it->second));
        }
        return nullptr;
    }

    GameSessionPointer FindGameSession(const Map::Id& map_id) {
        auto game_session = game_session_handler_.FindGameSession(map_id);
        if (!game_session) {
            game_session = CreateGameSession(FindMap(map_id));
        }

        return game_session_handler_.FindGameSession(map_id);
    }

   private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    GameSessionHandler game_session_handler_;

    GameSessionPointer CreateGameSession(Map::ConstPointer map) {
        return game_session_handler_.CreateGameSession(map);
    }
};

}  // namespace app
