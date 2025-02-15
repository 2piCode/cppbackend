#include "game_session_handler.h"

namespace app {

const GameSession::Pointer GameSessionHandler::FindGameSession(
    const model::Map::Id& map_id) const {
    if (auto it = map_id_to_game_session_.find(map_id);
        it != map_id_to_game_session_.end()) {
        // TODO: find the game session with the least amount of players
        return it->second[0];
    }
    return nullptr;
}

const GameSession::Pointer GameSessionHandler::CreateGameSession(
    model::Map::ConstPointer map) {
    GameSession new_session(map);
    auto game_session_ptr =
        game_sessions_.emplace_back(std::make_shared<app::GameSession>(map));
    map_id_to_game_session_[map->GetId()].push_back(game_session_ptr);
    return game_session_ptr;
}

}  // namespace app
