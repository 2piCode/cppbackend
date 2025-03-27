#include "application.h"

namespace app {
Application::Application(Players::Pointer players, Game::Pointer game,
                         loot_gen::LootGenerator::Pointer loot_generator,
                         LootHandler::Pointer loot_handler,
                         LootNumberMapHandler::Pointer loot_number_map_handler,
                         bool is_random_spawn_point,
                         std::shared_ptr<postgres::UnitOfWorkFactory> factory)
    : players_(players),
      game_(game),
      loot_generator_(loot_generator),
      loot_handler_(loot_handler),
      loot_number_map_handler_(loot_number_map_handler),
      is_random_spawn_point_(is_random_spawn_point),
      join_game_use_case_(game_, players_, is_random_spawn_point_),
      list_map_use_case_(game_),
      get_map_use_case_(game_, loot_handler_),
      list_player_use_case_(players_),
      get_game_state_use_case_(players_),
      move_player_use_case_(players_),
      game_tick_use_case_(game_, players_, loot_generator_, loot_handler_,
                          loot_number_map_handler_, factory),
      get_game_records_use_case_(factory) {}

Game::Maps Application::ListMaps() const {
    return list_map_use_case_.GetMaps();
}

GetMapResult Application::GetMap(const model::Map::Id& map_id) const {
    return get_map_use_case_.GetMap(map_id);
}

JoinGameResult Application::JoinGame(model::Map::Id map_id,
                                     const std::string& user_name) {
    return join_game_use_case_.Join(map_id, user_name);
}

ListPlayerResult Application::ListPlayers(const app::Token token) const {
    return list_player_use_case_.GetPlayers(token);
}

GameState Application::GetGameState(const app::Token token) const {
    return get_game_state_use_case_.GetGameState(token);
}

void Application::MovePlayer(const app::Token token,
                             const model::Direction direction) {
    return move_player_use_case_.MovePlayer(token, direction);
}

void Application::Tick(std::chrono::milliseconds delta_time) {
    game_tick_use_case_.Tick(delta_time);
    tick_signal_(delta_time);
}

boost::signals2::connection Application::DoOnTick(
    const TickSignal::slot_type& handler) {
    return tick_signal_.connect(handler);
}

std::vector<GameRecord> Application::GetGameRecords(int start, int max_items) {
    return get_game_records_use_case_.GetGameRecords(start, max_items);
}
}  // namespace app
