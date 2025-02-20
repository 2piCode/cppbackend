#include "game.h"

namespace app {

Game::Game(std::deque<Map> maps, double default_dog_speed)
    : maps_(std::move(maps)), default_dog_speed_(default_dog_speed) {
    for (size_t i = 0; i < maps_.size(); ++i) {
        map_id_to_index_[maps_[i].GetId()] = i;
    }
}

const model::Map::Pointer Game::FindMap(const Map::Id& id) const noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return std::make_shared<Map>(maps_.at(it->second));
    }
    return nullptr;
}

GameSession::Pointer Game::FindGameSession(const Map::Id& map_id) {
    auto game_session = game_session_handler_.FindGameSession(map_id);
    if (!game_session) {
        game_session = CreateGameSession(map_id);
    }

    return game_session;
}

GameSession::Pointer Game::CreateGameSession(const Map::Id& map_id) {
    auto map = FindMap(map_id);
    if (!map) {
        return nullptr;
    }

    return game_session_handler_.CreateGameSession(map);
}

}  // namespace app
