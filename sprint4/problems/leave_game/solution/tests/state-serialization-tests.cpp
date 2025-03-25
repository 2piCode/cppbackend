#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <deque>
#include <memory>
#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "../src/model/item.h"
#include "../src/model/model.h"
#include "../src/serialization/model_serialization.h"
#include "app/token.h"
#include "loots/loot_generator.h"
#include "loots/loot_number_map_handler.h"
#include "serialization/application_serialization.h"

using namespace model;
using namespace std::literals;
using namespace app;

namespace {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive output_archive{strm};
};

}  // namespace

class RoadVectorMatcher
    : public Catch::Matchers::MatcherBase<std::vector<Road::Pointer>> {
   public:
    explicit RoadVectorMatcher(const std::vector<Road::Pointer>& expected)
        : expected_{expected} {}

    bool match(const std::vector<Road::Pointer>& actual) const override {
        if (actual.size() != expected_.size()) {
            return false;
        }
        for (size_t i = 0; i < actual.size(); ++i) {
            if (actual[i]->GetStart() != expected_[i]->GetStart() ||
                actual[i]->GetEnd() != expected_[i]->GetEnd()) {
                return false;
            }
        }
        return true;
    }

    std::string describe() const override {
        return "matches two Road vectors by comparing each Road's start/end";
    }

   private:
    std::vector<Road::Pointer> expected_;
};

inline RoadVectorMatcher EqualsRoadVec(
    const std::vector<Road::Pointer>& roads) {
    return RoadVectorMatcher(roads);
}

void CheckDogEquality(const model::Dog& actual, const model::Dog& expected) {
    CHECK(*actual.GetId() == *expected.GetId());
    CHECK(actual.GetName() == expected.GetName());
    CHECK(actual.GetPosition() == expected.GetPosition());
    CHECK(actual.GetVelocity() == expected.GetVelocity());
    CHECK(actual.GetItemCount() == expected.GetItemCount());

    REQUIRE_THAT(actual.GetItems(),
                 Catch::Matchers::Equals(expected.GetItems()));
}

void CheckDogsEqualityDeque(const std::deque<model::Dog>& actual,
                            const std::deque<model::Dog>& expected) {
    CHECK(actual.size() == expected.size());
    for (size_t i = 0; i < actual.size(); i++) {
        CheckDogEquality(actual[i], expected[i]);
    }
}

void CheckMapEquality(const model::Map& actual, const model::Map& expected) {
    CHECK(actual.GetId() == expected.GetId());
    CHECK(actual.GetName() == expected.GetName());

    CHECK_THAT(actual.GetOffices(),
               Catch::Matchers::Equals(expected.GetOffices()));

    CHECK_THAT(actual.GetBuildings(),
               Catch::Matchers::Equals(expected.GetBuildings()));

    CHECK_THAT(actual.GetRoads(), EqualsRoadVec(expected.GetRoads()));

    CHECK(actual.GetMaxSpeed() == expected.GetMaxSpeed());
    CHECK(actual.GetNumberOfLootTypes() == expected.GetNumberOfLootTypes());
}

void CheckMapsEqualityDeque(const std::deque<Map::Pointer>& actual,
                            const std::deque<Map::Pointer>& expected) {
    CHECK(actual.size() == expected.size());
    for (size_t i = 0; i < actual.size(); i++) {
        CheckMapEquality(*actual[i], *expected[i]);
    }
}

void CheckGameSessionEquality(const app::GameSession& actual,
                              const app::GameSession& expected) {
    CHECK(actual.GetLootNumber() == expected.GetLootNumber());
    CHECK(actual.GetLootPositionsInfo() == expected.GetLootPositionsInfo());
    CheckMapEquality(*actual.GetMap(), *expected.GetMap());
    CheckDogsEqualityDeque(actual.GetDogs(), expected.GetDogs());
    CHECK(actual.GetLastItemId() == expected.GetLastItemId());
}

void CheckPlayerEquality(const app::Player& actual,
                         const app::Player& expected) {
    CHECK(*actual.GetId() == *expected.GetId());
    CHECK(actual.GetItemCount() == expected.GetItemCount());
    CheckDogEquality(*actual.GetDog(), *expected.GetDog());
    CheckGameSessionEquality(*actual.GetSession(), *expected.GetSession());
}

void CheckPlayersDequeEquality(const std::deque<app::Player>& actual,
                               const std::deque<app::Player>& expected) {
    CHECK(actual.size() == expected.size());
    for (size_t i = 0; i < actual.size(); i++) {
        CheckPlayerEquality(actual[i], expected[i]);
    }
}

void CheckListPlayerResultEquality(const ListPlayerResult& actual,
                                   const ListPlayerResult& expected) {
    CHECK(actual.player_infos.size() == expected.player_infos.size());
    for (size_t i = 0; i < actual.player_infos.size(); i++) {
        CHECK(*actual.player_infos[i].id == *expected.player_infos[i].id);
        CHECK(actual.player_infos[i].name == expected.player_infos[i].name);
    }
}

void CheckGameStateEquality(const GameState& actual,
                            const GameState& expected) {
    CHECK(actual.player_coord_infos.size() ==
          expected.player_coord_infos.size());
    CHECK(actual.lost_objects.size() == expected.lost_objects.size());
    for (size_t i = 0; i < actual.player_coord_infos.size(); i++) {
        const PlayerGameState &actual_player_game_state =
                                  actual.player_coord_infos[i],
                              expected_player_game_state =
                                  expected.player_coord_infos[i];
        CHECK(*actual_player_game_state.id == *expected_player_game_state.id);
        CHECK(actual_player_game_state.position ==
              expected_player_game_state.position);
        CHECK(actual_player_game_state.velocity ==
              expected_player_game_state.velocity);
        CHECK(actual_player_game_state.direction ==
              expected_player_game_state.direction);
        CHECK(actual_player_game_state.score ==
              expected_player_game_state.score);
        CHECK(actual_player_game_state.items.size() ==
              expected_player_game_state.items.size());
        for (size_t j = 0; j < actual_player_game_state.items.size(); j++) {
            CHECK(actual_player_game_state.items[j] ==
                  expected_player_game_state.items[j]);
        }
    }

    for (size_t i = 0; i < actual.lost_objects.size(); i++) {
        const model::Item &actual_item = actual.lost_objects[i],
                          expected_item = expected.lost_objects[i];
        CHECK(*actual_item.id == *expected_item.id);
        CHECK(actual_item.type == expected_item.type);
        CHECK(actual_item.value == expected_item.value);
        CHECK(actual_item.position == expected_item.position);
    }
}

SCENARIO_METHOD(Fixture, "Point serialization") {
    GIVEN("A point") {
        const model::Coordinate p{10, 20};
        WHEN("point is serialized") {
            output_archive << p;

            THEN("it is equal to point after serialization") {
                InputArchive input_archive{strm};
                model::Coordinate restored_point;
                input_archive >> restored_point;
                CHECK(p == restored_point);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Item serialization") {
    GIVEN("An item") {
        const model::Item item{.id = model::Item::Id{1},
                               .type = 2,
                               .position = {1.5, 2.5},
                               .value = 5};

        WHEN("item is serialized") {
            output_archive << item;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                model::Item deserialized_item;

                input_archive >> deserialized_item;

                CHECK(*deserialized_item.id == *item.id);
                CHECK(deserialized_item.type == item.type);
                CHECK(deserialized_item.value == item.value);
                CHECK(deserialized_item.position == item.position);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Rectangle serialization") {
    GIVEN("A rectangle") {
        const model::Rectangle rectangle{.position = {1, 2}, .size = {3, 4}};

        WHEN("rectangle is serialized") {
            output_archive << rectangle;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                model::Rectangle deserialized_rectangle;
                input_archive >> deserialized_rectangle;

                CHECK(deserialized_rectangle.position.x ==
                      rectangle.position.x);
                CHECK(deserialized_rectangle.position.y ==
                      rectangle.position.y);
                CHECK(deserialized_rectangle.size.width ==
                      rectangle.size.width);
                CHECK(deserialized_rectangle.size.height ==
                      rectangle.size.height);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Dog Serialization") {
    GIVEN("a dog") {
        const auto dog = [] {
            model::Dog dog{42, "Pluto"s, 3, {42.2, 12.5}};
            model::Item item = model::Item{.id = model::Item::Id{1},
                                           .type = 1,
                                           .position = model::Coordinate{0, 0},
                                           .value = 10};
            dog.AddItem(item);
            dog.DropAllItems();
            dog.AddItem(item);
            dog.SetDirection(Direction::EAST);
            return dog;
        }();

        WHEN("dog is serialized") {
            {
                serialization::DogRepr repr{dog};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::DogRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();

                CheckDogEquality(dog, restored);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Token serialization") {
    GIVEN("A token") {
        const Token token{"token_value"};

        WHEN("token is serialized") {
            output_archive << token;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                Token deserialized_token{""};
                input_archive >> deserialized_token;

                CHECK(*token == *deserialized_token);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Building serialization") {
    GIVEN("A building") {
        const model::Building building(
            model::Rectangle{.position = {1, 2}, .size = {3, 4}});

        WHEN("building is serialized") {
            serialization::BuildingRepr serialized_building(building);
            output_archive << serialized_building;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::BuildingRepr building_repr;
                input_archive >> building_repr;
                model::Building deserialized_building = building_repr.Restore();

                CHECK(deserialized_building == building);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Road serialization") {
    GIVEN("A road") {
        const model::Road road(model::Road::VERTICAL, {1, 2}, 4);

        WHEN("road is serialized") {
            serialization::RoadRepr serialized_road(road);
            output_archive << serialized_road;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::RoadRepr road_repr;
                input_archive >> road_repr;
                model::Road deserialized_road = road_repr.Restore();

                CHECK(deserialized_road == road);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Office serialization") {
    GIVEN("A office") {
        const model::Office office(model::Office::Id{"id"}, {2, 3}, {4, 5});

        WHEN("road is serialized") {
            serialization::OfficeRepr serialized_office(office);
            output_archive << serialized_office;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::OfficeRepr office_repr;
                input_archive >> office_repr;
                model::Office deserialized_office = office_repr.Restore();

                CHECK(deserialized_office == office);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Map serialization") {
    GIVEN("A map") {
        const Map map(Map::Id{"id"}, "name",
                      Map::Roads{std::make_shared<model::Road>(
                          model::Road::VERTICAL, model::Point{1, 2}, 4)},
                      Map::Buildings{model::Building{model::Rectangle{
                          .position = {1, 2}, .size = {3, 4}}}},
                      Map::Offices{model::Office{
                          model::Office::Id{"id"}, {2, 3}, {4, 5}}},
                      10, 11);

        WHEN("Map is serialized") {
            serialization::MapRepr serialized_map(map);
            output_archive << serialized_map;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::MapRepr map_repr;
                input_archive >> map_repr;
                Map::Pointer deserialized_map_ptr = map_repr.Restore();

                CheckMapEquality(map, *deserialized_map_ptr);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "GameSession serialization") {
    GIVEN("A game session") {
        const Map::Pointer map = std::make_shared<Map>(
            Map::Id{"id"}, "name",
            Map::Roads{std::make_shared<model::Road>(model::Road::VERTICAL,
                                                     model::Point{1, 2}, 4)},
            Map::Buildings{model::Building{
                model::Rectangle{.position = {1, 2}, .size = {3, 4}}}},
            Map::Offices{
                model::Office{model::Office::Id{"id"}, {2, 3}, {4, 5}}},
            10, 11);
        const GameSession game_session(map);

        WHEN("game session is serialized") {
            serialization::GameSessionRepr serialized_game_session(
                game_session);
            output_archive << serialized_game_session;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::GameSessionRepr game_session_repr;
                input_archive >> game_session_repr;
                GameSession deserialized_game_session =
                    game_session_repr.Restore(
                        [&map](const model::Map::Id) { return map; });
                CheckGameSessionEquality(deserialized_game_session,
                                         game_session);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "GameSessionHandler serialization") {
    GIVEN("A game session handler") {
        const Map::Pointer map = std::make_shared<Map>(
            Map::Id{"id"}, "name",
            Map::Roads{std::make_shared<model::Road>(model::Road::VERTICAL,
                                                     model::Point{1, 2}, 4)},
            Map::Buildings{model::Building{
                model::Rectangle{.position = {1, 2}, .size = {3, 4}}}},
            Map::Offices{
                model::Office{model::Office::Id{"id"}, {2, 3}, {4, 5}}},
            10, 11);
        GameSessionHandler game_session_handler;
        const GameSession::Pointer game_session_ptr =
            game_session_handler.CreateGameSession(map);

        WHEN("game session handler is serialized") {
            serialization::GameSessionHandlerRepr serialized_game_session(
                game_session_handler);
            output_archive << serialized_game_session;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::GameSessionHandlerRepr game_session_handler_repr;
                input_archive >> game_session_handler_repr;
                GameSessionHandler::Pointer deserialized_game_session_handler =
                    game_session_handler_repr.Restore(
                        [&map](const model::Map::Id) { return map; });

                CheckGameSessionEquality(
                    *deserialized_game_session_handler->FindGameSession(
                        map->GetId()),
                    *game_session_ptr);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Game serialization") {
    GIVEN("A game") {
        const Map::Pointer map = std::make_shared<Map>(
            Map::Id{"id"}, "name",
            Map::Roads{std::make_shared<model::Road>(model::Road::VERTICAL,
                                                     model::Point{1, 2}, 4)},
            Map::Buildings{model::Building{
                model::Rectangle{.position = {1, 2}, .size = {3, 4}}}},
            Map::Offices{
                model::Office{model::Office::Id{"id"}, {2, 3}, {4, 5}}},
            10, 11);

        Game game({map}, 1.0, std::make_shared<GameSessionHandler>());
        GameSession::Pointer game_session_ptr =
            game.CreateGameSession(map->GetId());

        WHEN("game is serialized") {
            serialization::GameRepr serialized_game(game);
            output_archive << serialized_game;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::GameRepr game_repr;
                input_archive >> game_repr;
                Game::Pointer deserialized_game = game_repr.Restore();

                CheckMapsEqualityDeque(game.GetMaps(),
                                       deserialized_game->GetMaps());
                CheckMapEquality(*game.FindMap(map->GetId()),
                                 *deserialized_game->FindMap(map->GetId()));
                CHECK(game.GetDefaultDogSpeed() ==
                      deserialized_game->GetDefaultDogSpeed());
                CheckGameSessionEquality(
                    *deserialized_game->FindGameSession(map->GetId()),
                    *game_session_ptr);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Player serialization") {
    GIVEN("A player") {
        const Map::Pointer map = std::make_shared<Map>(
            Map::Id{"id"}, "name",
            Map::Roads{std::make_shared<model::Road>(model::Road::VERTICAL,
                                                     model::Point{1, 2}, 4)},
            Map::Buildings{model::Building{
                model::Rectangle{.position = {1, 2}, .size = {3, 4}}}},
            Map::Offices{
                model::Office{model::Office::Id{"id"}, {2, 3}, {4, 5}}},
            10, 11);

        GameSession::Pointer game_session = std::make_shared<GameSession>(map);
        auto dog_ptr = [&game_session] {
            model::Dog::Pointer dog_ptr =
                game_session->AddDog({42.2, 12.5}, "Pluto"s, 42);
            model::Item item = model::Item{.id = model::Item::Id{1},
                                           .type = 1,
                                           .position = model::Coordinate{0, 0},
                                           .value = 10};
            dog_ptr->AddItem(item);
            dog_ptr->DropAllItems();
            dog_ptr->AddItem(item);
            dog_ptr->SetDirection(Direction::EAST);
            return dog_ptr;
        }();

        Player player(game_session, dog_ptr);

        WHEN("player is serialized") {
            serialization::PlayerRepr serialized_player(player);
            output_archive << serialized_player;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::PlayerRepr player_repr;
                input_archive >> player_repr;
                Player deserialized_player = player_repr.Restore(
                    [&game_session](const model::Map::Id map_id) {
                        return game_session;
                    });

                CheckPlayerEquality(deserialized_player, player);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Players serialization") {
    GIVEN("A players") {
        const Map::Pointer map = std::make_shared<Map>(
            Map::Id{"id"}, "name",
            Map::Roads{std::make_shared<model::Road>(model::Road::VERTICAL,
                                                     model::Point{1, 2}, 4)},
            Map::Buildings{model::Building{
                model::Rectangle{.position = {1, 2}, .size = {3, 4}}}},
            Map::Offices{
                model::Office{model::Office::Id{"id"}, {2, 3}, {4, 5}}},
            10, 11);

        GameSession::Pointer game_session = std::make_shared<GameSession>(map);
        auto dog_ptr = [&game_session] {
            model::Dog::Pointer dog_ptr =
                game_session->AddDog({42.2, 12.5}, "Pluto"s, 42);
            model::Item item = model::Item{.id = model::Item::Id{1},
                                           .type = 1,
                                           .position = model::Coordinate{0, 0},
                                           .value = 10};
            dog_ptr->AddItem(item);
            dog_ptr->DropAllItems();
            dog_ptr->AddItem(item);
            dog_ptr->SetDirection(Direction::EAST);
            return dog_ptr;
        }();

        Players players;
        auto [player_id, token] = players.Add(game_session, dog_ptr);

        WHEN("players is serialized") {
            serialization::PlayersRepr serialized_players(players);
            output_archive << serialized_players;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::PlayersRepr players_repr;
                input_archive >> players_repr;
                Players::Pointer deserialized_players_ptr =
                    players_repr.Restore(
                        [&game_session](const model::Map::Id map_id) {
                            return game_session;
                        });

                CheckPlayersDequeEquality(
                    deserialized_players_ptr->GetPlayers(),
                    players.GetPlayers());
                CheckPlayerEquality(*deserialized_players_ptr->Find(token),
                                    *players.Find(token));
                CheckPlayerEquality(
                    *deserialized_players_ptr->Find(
                        {dog_ptr->GetId(), map->GetId()}),
                    *players.Find({dog_ptr->GetId(), map->GetId()}));
                CHECK(*deserialized_players_ptr->FindToken(player_id) ==
                      *players.FindToken(player_id));
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Loot generator serialization") {
    GIVEN("A loot generator") {
        loot_gen::LootGenerator loot_generator(20ms, 0.1, 25ms);

        WHEN("players is serialized") {
            serialization::LootGeneratorRepr serialized_loot_generator(
                loot_generator);
            output_archive << serialized_loot_generator;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::LootGeneratorRepr loot_generator_repr;
                input_archive >> loot_generator_repr;
                loot_gen::LootGenerator::Pointer deserialized_loot_generator =
                    loot_generator_repr.Restore();

                CHECK(deserialized_loot_generator->GetBaseInterval() ==
                      loot_generator.GetBaseInterval());
                CHECK(deserialized_loot_generator->GetProbability() ==
                      loot_generator.GetProbability());
                CHECK(deserialized_loot_generator->GetTimeWithoutLoot() ==
                      loot_generator.GetTimeWithoutLoot());
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Loot handler serialization") {
    GIVEN("A loot handler") {
        model::Map::Id map_id = model::Map::Id("id");
        LootHandler loot_handler(
            LootHandler::LootTypeByMap{{map_id, boost::json::array{"Hello"}}},
            LootHandler::LootTypeScoreByMap{{map_id, {10}}});

        WHEN("players is serialized") {
            serialization::LootHandlerRepr serialized_loot_handler(
                loot_handler);
            output_archive << serialized_loot_handler;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::LootHandlerRepr loot_handler_repr;
                input_archive >> loot_handler_repr;
                LootHandler::Pointer deserialized_loot_handler =
                    loot_handler_repr.Restore();

                CHECK(deserialized_loot_handler->FindLootType(map_id) ==
                      loot_handler.FindLootType(map_id));
                CHECK(
                    deserialized_loot_handler->FindValueByLootType(map_id, 0) ==
                    loot_handler.FindValueByLootType(map_id, 0));
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Loot number map handler serialization") {
    GIVEN("A loot number map handler") {
        model::Map::Id map_id = model::Map::Id("id");
        LootNumberMapHandler loot_number_map_handler(
            LootNumberMapHandler::LootNumberByMap{{map_id, 10}}, 5);

        WHEN("players is serialized") {
            serialization::LootNumberMapHandlerRepr
                serialized_loot_number_map_handler(loot_number_map_handler);
            output_archive << serialized_loot_number_map_handler;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::LootNumberMapHandlerRepr
                    loot_number_map_handler_repr;
                input_archive >> loot_number_map_handler_repr;
                LootNumberMapHandler::Pointer
                    deserialized_loot_number_map_handler =
                        loot_number_map_handler_repr.Restore();

                CHECK(deserialized_loot_number_map_handler->GetMaxLootNumber(
                          map_id) ==
                      loot_number_map_handler.GetMaxLootNumber(map_id));
                CHECK(deserialized_loot_number_map_handler->GetMaxLootNumber(
                          model::Map::Id("zero_id")) ==
                      loot_number_map_handler.GetMaxLootNumber(
                          model::Map::Id("zero_id")));
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Application serialization") {
    GIVEN("An application") {
        const Map::Pointer map = std::make_shared<Map>(
            Map::Id{"id"}, "name",
            Map::Roads{std::make_shared<model::Road>(model::Road::VERTICAL,
                                                     model::Point{1, 2}, 4)},
            Map::Buildings{model::Building{
                model::Rectangle{.position = {1, 2}, .size = {3, 4}}}},
            Map::Offices{
                model::Office{model::Office::Id{"id"}, {2, 3}, {4, 5}}},
            10, 11);

        Game::Pointer game = std::make_shared<Game>(
            Game::Maps{map}, 1.0, std::make_shared<GameSessionHandler>());
        GameSession::Pointer game_session_ptr =
            game->CreateGameSession(map->GetId());
        game_session_ptr->AddLoot(1, {2.1, 3.1}, 10);

        auto dog_ptr = [&game_session_ptr] {
            model::Dog::Pointer dog_ptr =
                game_session_ptr->AddDog({42.2, 12.5}, "Pluto"s, 42);
            model::Item item = model::Item{.id = model::Item::Id{1},
                                           .type = 1,
                                           .position = model::Coordinate{0, 0},
                                           .value = 10};
            dog_ptr->AddItem(item);
            dog_ptr->DropAllItems();
            dog_ptr->AddItem(item);
            dog_ptr->SetDirection(Direction::EAST);
            return dog_ptr;
        }();

        Players::Pointer players = std::make_shared<Players>();
        auto [player_id, token] = players->Add(game_session_ptr, dog_ptr);

        loot_gen::LootGenerator::Pointer loot_generator =
            std::make_shared<loot_gen::LootGenerator>(20ms, 0.1, 25ms);

        LootHandler::Pointer loot_handler = std::make_shared<LootHandler>(
            LootHandler::LootTypeByMap{
                {map->GetId(), boost::json::array{"Hello"}}},
            LootHandler::LootTypeScoreByMap{{map->GetId(), {10}}});

        LootNumberMapHandler::Pointer loot_number_map_handler =
            std::make_shared<LootNumberMapHandler>(
                LootNumberMapHandler::LootNumberByMap{{map->GetId(), 10}}, 5);

        Application application(players, game, loot_generator, loot_handler,
                                loot_number_map_handler, true, nullptr);

        WHEN("application is serialized") {
            serialization::ApplicationRepr serialized_application(application);
            output_archive << serialized_application;

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::ApplicationRepr application_repr;
                input_archive >> application_repr;
                Application::Pointer deserialized_application =
                    application_repr.Restore();

                auto deserialized_get_map_result =
                    deserialized_application->GetMap(map->GetId());
                auto application_get_map_result =
                    application.GetMap(map->GetId());
                CHECK(deserialized_get_map_result.loot_types ==
                      application_get_map_result.loot_types);
                CheckMapEquality(*deserialized_get_map_result.map_ptr,
                                 *application_get_map_result.map_ptr);
                CheckMapsEqualityDeque(deserialized_application->ListMaps(),
                                       application.ListMaps());
                CheckListPlayerResultEquality(
                    deserialized_application->ListPlayers(token),
                    application.ListPlayers(token));
                CheckGameStateEquality(
                    deserialized_application->GetGameState(token),
                    application.GetGameState(token));
            }
        }
    }
}
