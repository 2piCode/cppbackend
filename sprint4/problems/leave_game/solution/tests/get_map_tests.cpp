#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <utility>

#include "app/game/game_session_handler.h"
#include "app/use_cases/get_map_use_case.h"
#include "data/test_games_data.h"

SCENARIO("Get map tests") {
    GIVEN("game with zero map") {
        app::Game::Pointer game = std::make_shared<EmptyGame>(
            app::Game::Maps{}, 1.0,
            std::make_shared<app::GameSessionHandler>());
        GetMapUseCase use_case(game, nullptr);
        CHECK_THROWS_AS(use_case.GetMap(model::Map::Id{""}), GetMapError);
    }

    GIVEN("game with correct map") {
        std::shared_ptr<GameWithOneMap> game = std::make_shared<GameWithOneMap>(
            app::Game::Maps{}, 1.0,
            std::make_shared<app::GameSessionHandler>());

        AND_GIVEN("empty loot handler") {
            GetMapUseCase use_case(game,
                                   std::make_shared<LootHandler>(
                                       LootHandler::LootTypeByMap{},
                                       LootHandler::LootTypeScoreByMap{}));
            CHECK_THROWS_AS(use_case.GetMap(model::Map::Id{""}), GetMapError);
        }

        AND_GIVEN("loot types array with map") {
            boost::json::array loot_types_array =
                boost::json::array{{{"name", "key"},
                                    {"file", "assets/key.obj"},
                                    {"type", "obj"},
                                    {"rotation", 90},
                                    {"color", "#338844"},
                                    {"scale", 0.03}},
                                   {{"name", "wallet"},
                                    {"file", "assets/wallet.obj"},
                                    {"type", "obj"},
                                    {"rotation", 0},
                                    {"color", "#883344"},
                                    {"scale", 0.01}}};
            GetMapUseCase use_case(
                game,
                std::make_shared<LootHandler>(
                    LootHandler::LootTypeByMap{
                        {game->GetCorrectMap()->GetId(), loot_types_array}},
                    LootHandler::LootTypeScoreByMap{}));

            auto get_map_result =
                use_case.GetMap(game->GetCorrectMap()->GetId());
            CHECK(get_map_result.map_ptr == game->GetCorrectMap());
            CHECK(get_map_result.loot_types == loot_types_array);
        }
    }
}
