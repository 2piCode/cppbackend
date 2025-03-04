#include "game.h"

#include <memory>
#include <utility>

namespace app {

Game::Game(Maps maps, double default_dog_speed)
    : maps_(std::move(maps)), default_dog_speed_(default_dog_speed) {
    for (size_t i = 0; i < maps_.size(); ++i) {
        map_id_to_index_[maps_[i]->GetId()] = i;
    }
}

const model::Map::Pointer Game::FindMap(const Map::Id& id) const noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return maps_.at(it->second);
    }
    return nullptr;
}

GameSession::Pointer Game::FindGameSession(const Map::Id& map_id) {
    return game_session_handler_.FindGameSession(map_id);
}

GameSession::Pointer Game::CreateGameSession(const Map::Id& map_id) {
    auto map = FindMap(map_id);
    if (!map) {
        return nullptr;
    }

    return game_session_handler_.CreateGameSession(map);
}

}  // namespace app
