#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <memory>
#include <ratio>

#include "app/player/player.h"
#include "app/player/players.h"
#include "app/use_cases/game_tick_use_case.h"
#include "data/test_games_data.h"
#include "data/test_players_data.h"
#include "loots/loot_generator.h"
#include "loots/loot_number_map_handler.h"
#include "utils/logger.h"

SCENARIO("Tick tests") {
    GIVEN("empty players and empty game") {
        std::shared_ptr<app::PlayersCollection> players =
            std::make_shared<test_players::EmptyPlayers>();
        AND_GIVEN("empty game") {
            std::shared_ptr<EmptyGame> game = std::make_shared<EmptyGame>(
                app::Game::Maps{}, 10.0,
                std::make_shared<app::GameSessionHandler>());
            GameTickUseCase use_case(game, players, nullptr, nullptr, nullptr);

            WHEN("delta time is zero or less") {
                CHECK_THROWS_AS(use_case.Tick(std::chrono::milliseconds(0)),
                                GameTickError);
                CHECK_THROWS_AS(use_case.Tick(std::chrono::milliseconds(-1)),
                                GameTickError);
            }
        }

        AND_GIVEN("game with one map") {
            std::shared_ptr<GameWithOneMap> game =
                std::make_shared<GameWithOneMap>(
                    app::Game::Maps{}, 10.0,
                    std::make_shared<app::GameSessionHandler>());

            AND_GIVEN("loot generator with 1 milliseconds timing") {
                GameTickUseCase use_case(
                    game, players,
                    std::make_unique<loot_gen::LootGenerator>(
                        std::chrono::milliseconds(1), 1.0,
                        std::chrono::milliseconds(2)),
                    std::make_shared<LootHandler>(
                        LootHandler::LootTypeByMap{},
                        LootHandler::LootTypeScoreByMap{}),
                    std::make_shared<LootNumberMapHandler>(
                        LootNumberMapHandler::LootNumberByMap{}, 0));
                /*use_case.Tick(std::chrono::milliseconds(1));*/

                /*auto session = game->GetSession();*/
                /*CHECK(session->GetLootNumber() == 1);*/
                /**/
                /*use_case.Tick(std::chrono::milliseconds(1));*/
                /*CHECK(session->GetLootNumber() == 1);*/
            }
        }
    }
}
