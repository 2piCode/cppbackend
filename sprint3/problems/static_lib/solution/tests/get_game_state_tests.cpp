#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <memory>
#include <string>

#include "app/player/player.h"
#include "app/use_cases/get_game_state_use_case.h"
#include "data/test_players_data.h"

class PlayerGameStateMatcher
    : public Catch::Matchers::MatcherBase<PlayerGameState> {
   public:
    explicit PlayerGameStateMatcher(const PlayerGameState& expected)
        : expected_(expected) {}

    bool match(const PlayerGameState& actual) const override {
        if (actual.id != expected_.id) return false;
        if (actual.direction != expected_.direction) return false;

        if (actual.velocity.x != expected_.velocity.x ||
            actual.position.y != expected_.position.y) {
            return false;
        }

        if (actual.position.x != expected_.position.x ||
            actual.position.y != expected_.position.y) {
            return false;
        }

        return true;
    }

    std::string describe() const override {
        return "is not matching expected PlayerGameState";
    }

   private:
    PlayerGameState expected_;
};

inline PlayerGameStateMatcher EqualsPlayerGameState(
    const PlayerGameState& expected) {
    return PlayerGameStateMatcher(expected);
}

SCENARIO("Get game state") {
    GIVEN("empty players") {
        auto players = std::make_shared<test_players::EmptyPlayers>();
        GetGameStateUseCase use_case(players);
        THEN("should throw runtime error") {
            CHECK_THROWS_AS(use_case.GetGameState(app::Token{""}),
                            GetGameStateError);
        }
    }

    GIVEN("one joined player") {
        auto players = std::make_shared<test_players::OnePlayerPlayers>();
        GetGameStateUseCase use_case(players);

        WHEN("use get game state case") {
            auto game_state = use_case.GetGameState(app::Token{""});
            THEN("only one player info") {
                CHECK(game_state.player_coord_infos.size() == 1);
            }

            AND_THEN("player info equals correct") {
                auto first_player_info = game_state.player_coord_infos[0];
                auto correct_first_player_info = players->GetPlayerGameState();
                CHECK_THAT(first_player_info,
                           EqualsPlayerGameState(correct_first_player_info));
            }

            AND_THEN("don't have loot") {
                CHECK(game_state.lost_objects.size() == 0);
            }
        }

        AND_GIVEN("session with loot") {
            auto session = players->GetSession();
            int loot_type = 1;
            const auto position = model::Coordinate{0.0, 0.0};
            session->AddLoot(loot_type, position);

            WHEN("use get game state case") {
                auto game_state = use_case.GetGameState(app::Token{""});
                THEN("return only one loot item") {
                    CHECK(game_state.lost_objects.size() == 1);
                }

                AND_THEN("return right type for loot item") {
                    CHECK(game_state.lost_objects[0].type == loot_type);
                }

                AND_THEN("return right position for loot item") {
                    CHECK(game_state.lost_objects[0].position.x == position.x);
                    CHECK(game_state.lost_objects[0].position.y == position.y);
                }
            }
        }
    }
}
