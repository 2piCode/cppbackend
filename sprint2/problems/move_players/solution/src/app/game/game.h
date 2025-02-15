#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include "app/game/game_session_handler.h"
#include "model/model.h"

namespace app {

class Game {
   public:
    using Pointer = std::shared_ptr<Game>;

    using Map = model::Map;
    using Maps = std::deque<model::Map>;
    using GameSessionPointer = GameSession::Pointer;

    Game(std::deque<Map> maps, double default_dog_speed);

    const Maps& GetMaps() const noexcept { return maps_; }

    Map::ConstPointer FindMap(const Map::Id& id) const noexcept;

    GameSessionPointer FindGameSession(const Map::Id& map_id);

    double GetDefaultDogSpeed() const noexcept { return default_dog_speed_; }

   private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    GameSessionHandler game_session_handler_;
    double default_dog_speed_;

    GameSessionPointer CreateGameSession(const Map::Id& map_id);
};

}  // namespace app
