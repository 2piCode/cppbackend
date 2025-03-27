#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <boost/signals2.hpp>

#include "app/game/game.h"
#include "app/use_cases/game_tick_use_case.h"
#include "app/use_cases/get_game_records_use_case.h"
#include "app/use_cases/get_game_state_use_case.h"
#include "app/use_cases/get_map_use_case.h"
#include "app/use_cases/join_game_use_case.h"
#include "app/use_cases/list_map_use_case.h"
#include "app/use_cases/list_player_use_case.h"
#include "app/use_cases/move_player.h"
#include "loots/loot_generator.h"
#include "loots/loot_handler.h"
#include "loots/loot_number_map_handler.h"
#include "postgres/unit_of_work.h"

namespace serialization {
class ApplicationRepr;
}

namespace app {

class Application {
   public:
    friend class serialization::ApplicationRepr;

    using Pointer = std::shared_ptr<Application>;
    using TickSignal =
        boost::signals2::signal<void(std::chrono::milliseconds delta)>;

    explicit Application(Players::Pointer players, Game::Pointer game,
                         loot_gen::LootGenerator::Pointer loot_generator,
                         LootHandler::Pointer loot_handler,
                         LootNumberMapHandler::Pointer loot_number_map_handler,
                         bool is_random_spawn_point,
                         std::shared_ptr<postgres::UnitOfWorkFactory> factory);

    Game::Maps ListMaps() const;

    GetMapResult GetMap(const model::Map::Id& map_id) const;
    JoinGameResult JoinGame(model::Map::Id map_id,
                            const std::string& user_name);

    ListPlayerResult ListPlayers(const app::Token token) const;

    GameState GetGameState(const app::Token token) const;

    void MovePlayer(const app::Token token, const model::Direction direction);

    void Tick(std::chrono::milliseconds delta_time);

    [[nodiscard]] boost::signals2::connection DoOnTick(
        const TickSignal::slot_type& handler);

    std::vector<GameRecord> GetGameRecords(int start, int max_items);

   private:
    Players::Pointer players_;
    Game::Pointer game_;
    loot_gen::LootGenerator::Pointer loot_generator_;
    LootHandler::Pointer loot_handler_;
    LootNumberMapHandler::Pointer loot_number_map_handler_;
    bool is_random_spawn_point_;

    JoinGameUseCase join_game_use_case_;
    ListMapUseCase list_map_use_case_;
    GetMapUseCase get_map_use_case_;
    ListPlayerUseCase list_player_use_case_;
    GetGameStateUseCase get_game_state_use_case_;
    MovePlayerUseCase move_player_use_case_;
    GameTickUseCase game_tick_use_case_;
    GetGameRecordsUseCase get_game_records_use_case_;

    TickSignal tick_signal_;
};

}  // namespace app
