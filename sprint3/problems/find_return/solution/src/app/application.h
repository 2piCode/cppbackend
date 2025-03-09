#pragma once

#include <memory>
#include <string>
#include <utility>

#include "app/game/game.h"
#include "app/use_cases/game_tick_use_case.h"
#include "app/use_cases/get_game_state_use_case.h"
#include "app/use_cases/get_map_use_case.h"
#include "app/use_cases/join_game_use_case.h"
#include "app/use_cases/list_map_use_case.h"
#include "app/use_cases/list_player_use_case.h"
#include "app/use_cases/move_player.h"
#include "loots/loot_generator.h"
#include "loots/loot_handler.h"
#include "loots/loot_number_map_handler.h"

namespace app {

class Application {
   public:
    using Pointer = std::unique_ptr<Application>;
    explicit Application(
        Game::Pointer game, bool is_random_spawn_point,
        loot_gen::LootGenerator::Pointer&& loot_generator,
        LootHandler::Pointer&& loot_handler,
        LootNumberMapHandler::Pointer&& loot_number_map_handler)
        : game_(game),
          join_game_use_case_(game_, players_, is_random_spawn_point),
          list_map_use_case_(game_),
          get_map_use_case_(game_, loot_handler),
          list_player_use_case_(players_),
          get_game_state_use_case_(players_),
          move_player_use_case_(players_),
          game_tick_use_case_(game_, players_, std::move(loot_generator),
                              std::move(loot_number_map_handler)) {}

    Game::Maps ListMaps() const { return list_map_use_case_.GetMaps(); }

    GetMapResult GetMap(const model::Map::Id& map_id) const {
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

    void MovePlayer(const app::Token token, const model::Direction direction) {
        return move_player_use_case_.MovePlayer(token, direction);
    }

    void Tick(std::chrono::milliseconds delta_time) {
        game_tick_use_case_.Tick(delta_time);
    }

   private:
    std::shared_ptr<PlayersCollection> players_ = std::make_shared<Players>();
    Game::Pointer game_;
    JoinGameUseCase join_game_use_case_;
    ListMapUseCase list_map_use_case_;
    GetMapUseCase get_map_use_case_;
    ListPlayerUseCase list_player_use_case_;
    GetGameStateUseCase get_game_state_use_case_;
    MovePlayerUseCase move_player_use_case_;
    GameTickUseCase game_tick_use_case_;
};

}  // namespace app
