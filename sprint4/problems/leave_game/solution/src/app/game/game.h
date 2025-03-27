#pragma once

#include <chrono>
#include <deque>
#include <memory>
#include <ratio>
#include <unordered_map>

#include "app/game/game_session_handler.h"

namespace serialization {
class GameRepr;
}

namespace app {

namespace detail {
using namespace std::chrono_literals;

static constexpr std::chrono::milliseconds MILLISECONDS_PER_SECOND = 1000ms;

static constexpr std::chrono::milliseconds DEFAULT_DOG_RETIREMENT_TIME =
    60000ms;
}  // namespace detail

class Game {
   public:
    friend class serialization::GameRepr;

    using Pointer = std::shared_ptr<Game>;
    using Map = model::Map;
    using Maps = std::deque<std::shared_ptr<model::Map>>;
    using GameSessionPointer = GameSession::Pointer;

    Game(Maps maps, double default_dog_speed,
         GameSessionHandler::Pointer handler,
         std::chrono::milliseconds dog_retirement_time =
             detail::DEFAULT_DOG_RETIREMENT_TIME);
    virtual ~Game() = default;

    virtual const Maps& GetMaps() const noexcept { return maps_; }

    virtual const Map::Pointer FindMap(const Map::Id& id) const noexcept;

    GameSessionPointer CreateGameSession(const Map::Id& map_id);

    virtual GameSessionPointer FindGameSession(const Map::Id& map_id);

    double GetDefaultDogSpeed() const noexcept { return default_dog_speed_; }

    std::chrono::milliseconds GetDogRetirementTime() const noexcept {
        return dog_retirement_time_;
    }

   private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    GameSessionHandler::Pointer game_session_handler_;
    double default_dog_speed_;
    std::chrono::milliseconds dog_retirement_time_;
};

}  // namespace app
