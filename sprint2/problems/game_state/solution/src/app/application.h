#pragma once

#include <string>

#include "app/use_cases/get_game_state_use_case.h"
#include "app/use_cases/get_map_use_case.h"
#include "app/use_cases/join_game_use_case.h"
#include "app/use_cases/list_map_use_case.h"
#include "app/use_cases/list_player_use_case.h"
#include "game.h"

namespace app {

class Application {
   public:
    explicit Application(Game::Pointer game)
        : game_(game),
          join_game_use_case_(game_, players_),
          list_map_use_case_(game_),
          get_map_use_case_(game_),
          list_player_use_case_(players_),
          get_game_state_use_case_(players_) {}

    Game::Maps ListMaps() const { return list_map_use_case_.GetMaps(); }

    model::Map::ConstPointer GetMap(const model::Map::Id& map_id) const {
        return get_map_use_case_.GetMap(map_id);
    }

    JoinGameResult JoinGame(model::Map::Id map_id,
                            const std::string& user_name) {
        return join_game_use_case_.Join(map_id, user_name);
    }

    ListPlayerResult ListPlayers(const app::Token token) const {
        return list_player_use_case_.GetPlayers(token);
    }

    GameState GetGameState(const app::Token token) const {
        return get_game_state_use_case_.GetGameState(token);
    }

   private:
    Players players_;
    Game::Pointer game_;
    JoinGameUseCase join_game_use_case_;
    ListMapUseCase list_map_use_case_;
    GetMapUseCase get_map_use_case_;
    ListPlayerUseCase list_player_use_case_;
    GetGameStateUseCase get_game_state_use_case_;
};

}  // namespace app
