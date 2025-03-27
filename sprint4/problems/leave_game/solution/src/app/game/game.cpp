#include "game.h"

#include <chrono>
#include <utility>

namespace app {

Game::Game(Maps maps, double default_dog_speed,
           GameSessionHandler::Pointer handler,
           std::chrono::milliseconds dog_retirement_time)
    : maps_(std::move(maps)),
      game_session_handler_(handler),
      default_dog_speed_(default_dog_speed),
      dog_retirement_time_(dog_retirement_time) {
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
    return game_session_handler_->FindGameSession(map_id);
}

GameSession::Pointer Game::CreateGameSession(const Map::Id& map_id) {
    auto map = FindMap(map_id);
    if (!map) {
        return nullptr;
    }

    return game_session_handler_->CreateGameSession(map);
}

}  // namespace app
