#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/json/parse.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include "app/application.h"
#include "app/game/game_session.h"
#include "app/player/players.h"
#include "loots/loot_generator.h"
#include "loots/loot_handler.h"
#include "loots/loot_number_map_handler.h"
#include "model/model.h"
#include "serialization/model_serialization.h"

namespace serialization {

class BuildingRepr {
   public:
    BuildingRepr() = default;

    explicit BuildingRepr(const model::Building& building)
        : bounds_(building.GetBounds()) {}

    template <typename Archive>
    void serialize(Archive& archive, [[maybe_unused]] const unsigned version) {
        archive & bounds_;
    }

    model::Building Restore() const { return model::Building(bounds_); }

   private:
    model::Rectangle bounds_;
};

class RoadRepr {
   public:
    RoadRepr() = default;

    explicit RoadRepr(const model::Road& road)
        : start_(road.GetStart()), end_(road.GetEnd()) {}

    template <typename Archive>
    void serialize(Archive& archive, [[maybe_unused]] const unsigned version) {
        archive & start_;
        archive & end_;
    }

    model::Road Restore() const {
        if (start_.y == end_.y) {
            return model::Road(model::Road::HORIZONTAL, start_, end_.x);
        }
        return model::Road(model::Road::VERTICAL, start_, end_.y);
    }

   private:
    model::Point start_;
    model::Point end_;
};

class OfficeRepr {
   public:
    OfficeRepr() = default;

    explicit OfficeRepr(const model::Office& office)
        : id_(office.GetId()),
          position_(office.GetPosition()),
          offset_(office.GetOffset()) {}

    template <typename Archive>
    void serialize(Archive& archive, [[maybe_unused]] const unsigned version) {
        archive & id_;
        archive & position_;
        archive & offset_;
    }

    model::Office Restore() const {
        return model::Office(id_, position_, offset_);
    }

   private:
    model::Office::Id id_;
    model::Point position_;
    model::Offset offset_;
};

class MapRepr {
   public:
    MapRepr() = default;

    MapRepr(const model::Map& map)
        : id_(map.GetId()),
          name_(map.GetName()),
          dog_speed_(*map.GetMaxSpeed()),
          number_loot_types_(map.GetNumberOfLootTypes()) {
        for (const model::Road::Pointer road : map.GetRoads()) {
            roads_.emplace_back(*road);
        }

        for (const model::Building& building : map.GetBuildings()) {
            buildings_.emplace_back(building);
        }

        for (const model::Office& office : map.GetOffices()) {
            offices_.emplace_back(office);
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & id_;
        ar & name_;
        ar & roads_;
        ar & buildings_;
        ar & offices_;
        ar & dog_speed_;
        ar & number_loot_types_;
    }

    model::Map::Pointer Restore() const {
        return std::make_shared<model::Map>(
            id_, name_, RestoreRoads(), RestoreBuildings(), RestoreOffices(),
            dog_speed_, number_loot_types_);
    }

   private:
    model::Map::Id id_;
    std::string name_;
    std::vector<serialization::RoadRepr> roads_;
    std::vector<serialization::BuildingRepr> buildings_;
    std::vector<serialization::OfficeRepr> offices_;
    double dog_speed_ = 0.0;
    int number_loot_types_ = {};

    model::Map::Roads RestoreRoads() const {
        model::Map::Roads roads;
        for (const auto& road : roads_) {
            roads.push_back(std::make_shared<model::Road>(road.Restore()));
        }
        return roads;
    }

    model::Map::Buildings RestoreBuildings() const {
        model::Map::Buildings buildings;
        for (const auto& building : buildings_) {
            buildings.push_back(building.Restore());
        }
        return buildings;
    }

    model::Map::Offices RestoreOffices() const {
        model::Map::Offices offices;
        for (const auto& office : offices_) {
            offices.push_back(office.Restore());
        }
        return offices;
    }
};

class GameSessionRepr {
   public:
    GameSessionRepr() = default;

    explicit GameSessionRepr(const app::GameSession& session)
        : map_id_(session.GetMapId()), last_item_id_(session.GetLastItemId()) {
        for (const auto& dog : session.GetDogs()) {
            dogs_.emplace_back(dog);
        }
        for (const auto& item : session.GetLootPositionsInfo()) {
            items_.push_back(item);
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* map_id_;
        ar & dogs_;
        ar & items_;
        ar & last_item_id_;
    }

    app::GameSession Restore(
        std::function<model::Map::Pointer(model::Map::Id)> map_finder) const {
        app::GameSession game_session(map_finder(map_id_), last_item_id_);
        for (const auto& dog_repr : dogs_) {
            auto dog = dog_repr.Restore();
            game_session.dogs_.push_back(dog);
        }
        for (const auto& item : items_) {
            game_session.loot_positions_.push_back(item);
        }
        return game_session;
    }

   private:
    model::Map::Id map_id_;
    std::vector<serialization::DogRepr> dogs_;
    app::GameSession::LootPositionsVector items_;
    std::uint32_t last_item_id_;
};

class GameSessionHandlerRepr {
   public:
    GameSessionHandlerRepr() = default;

    GameSessionHandlerRepr(const app::GameSessionHandler& handler) {
        for (auto session_ptr : handler.game_sessions_) {
            sessions_.emplace_back(*session_ptr);
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & sessions_;
    }

    app::GameSessionHandler::Pointer Restore(
        std::function<model::Map::Pointer(model::Map::Id)> map_finder) const {
        app::GameSessionHandler::Pointer handler =
            std::make_shared<app::GameSessionHandler>();
        for (const auto& session_repr : sessions_) {
            const auto session = session_repr.Restore(map_finder);
            auto game_session_ptr = handler->game_sessions_.emplace_back(
                std::make_shared<app::GameSession>(session));
            handler->map_id_to_game_session_[session.GetMap()->GetId()]
                .push_back(game_session_ptr);
        }
        return handler;
    }

   private:
    std::vector<serialization::GameSessionRepr> sessions_;
};

class GameRepr {
   public:
    GameRepr() = default;

    explicit GameRepr(const app::Game& game)
        : handler_(*game.game_session_handler_),
          default_dog_speed_(game.default_dog_speed_) {
        for (const auto& map : game.maps_) {
            maps_.emplace_back(*map);
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & maps_;
        ar & handler_;
        ar & default_dog_speed_;
    }

    app::Game::Pointer Restore() const {
        app::Game::Maps maps;
        for (const auto& map : maps_) {
            maps.push_back(map.Restore());
        }
        auto map_finder = [&maps](const model::Map::Id& map_id) {
            for (const auto& map : maps) {
                if (map->GetId() == map_id) {
                    return map;
                }
            }
            throw std::runtime_error("Can't find map with this id: " + *map_id);
        };
        return std::make_shared<app::Game>(
            maps, default_dog_speed_, handler_.Restore(std::move(map_finder)));
    }

   private:
    std::vector<serialization::MapRepr> maps_;
    serialization::GameSessionHandlerRepr handler_;
    double default_dog_speed_;
};

class PlayerRepr {
   public:
    PlayerRepr() = default;

    explicit PlayerRepr(const app::Player& player)
        : game_session_map_id_(player.GetSession()->GetMapId()),
          dog_id_(player.GetId()) {}

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* game_session_map_id_;
        ar&* dog_id_;
    }

    app::Player Restore(std::function<app::GameSession::Pointer(model::Map::Id)>
                            game_session_finder) const {
        app::GameSession::Pointer session_ptr =
            game_session_finder(game_session_map_id_);
        const auto& dog_it = std::find_if(
            session_ptr->GetDogs().begin(), session_ptr->GetDogs().end(),
            [this](const model::Dog& dog) { return dog.GetId() == dog_id_; });
        model::Dog::Pointer dog = &const_cast<model::Dog&>(*dog_it);
        return app::Player(session_ptr, dog);
    }

   private:
    model::Map::Id game_session_map_id_;
    model::Dog::Id dog_id_;
};

class PlayersRepr {
   public:
    PlayersRepr() = default;

    explicit PlayersRepr(const app::Players& players)
        : tokens_(players.tokens_) {
        for (const auto& player : players.players_) {
            players_.emplace_back(player);
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & players_;
        ar & tokens_;
    }
    [[nodiscard]] app::Players::Pointer Restore(
        std::function<app::GameSession::Pointer(model::Map::Id)>
            game_session_finder) const {
        app::Players::Pointer players = std::make_shared<app::Players>();
        for (const auto& player_repr : players_) {
            players->players_.push_back(
                player_repr.Restore(game_session_finder));
        }

        players->tokens_ = tokens_;
        for (size_t i = 0; i < players_.size(); i++) {
            auto& player = players->players_[i];
            auto& token = players->tokens_[i];
            players->session_to_player_.emplace(
                app::PlayerSession{player.GetId(),
                                   player.GetSession()->GetMap()->GetId()},
                &player);
            players->token_to_player_.emplace(*token, &player);
            players->player_to_token_.emplace(player.GetId(), &token);
        }
        return players;
    }

   private:
    std::deque<serialization::PlayerRepr> players_;
    std::deque<app::Token> tokens_;
};

class LootGeneratorRepr {
   public:
    LootGeneratorRepr() = default;

    LootGeneratorRepr(const loot_gen::LootGenerator& generator)
        : base_interval_(generator.GetBaseInterval().count()),
          probability_(generator.GetProbability()),
          time_without_loot_(generator.GetTimeWithoutLoot().count()) {}

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & base_interval_;
        ar & probability_;
        ar & time_without_loot_;
    }

    loot_gen::LootGenerator::Pointer Restore() const {
        return std::make_shared<loot_gen::LootGenerator>(
            loot_gen::LootGenerator::TimeInterval(base_interval_), probability_,
            loot_gen::LootGenerator::TimeInterval(time_without_loot_));
    }

   private:
    int base_interval_;
    double probability_;
    int time_without_loot_;
};

class LootHandlerRepr {
   public:
    LootHandlerRepr() = default;

    LootHandlerRepr(const LootHandler& loot_handler)
        : map_score_types_(loot_handler.map_score_types_) {
        for (const auto& [id, json] : loot_handler.map_loot_types_) {
            map_loot_types_[id] = boost::json::serialize(json);
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & map_loot_types_;
        ar & map_score_types_;
    }

    LootHandler::Pointer Restore() const {
        LootHandler::LootTypeByMap loot_type;
        for (const auto& [id, json_str] : map_loot_types_) {
            loot_type[id] = boost::json::parse(json_str).as_array();
        }
        return std::make_shared<LootHandler>(loot_type, map_score_types_);
    }

   private:
    std::unordered_map<model::Map::Id, std::string,
                       util::TaggedHasher<model::Map::Id>>
        map_loot_types_;
    std::unordered_map<model::Map::Id, std::vector<int>,
                       util::TaggedHasher<model::Map::Id>>
        map_score_types_;
};

class LootNumberMapHandlerRepr {
   public:
    LootNumberMapHandlerRepr() = default;

    explicit LootNumberMapHandlerRepr(const LootNumberMapHandler& handler)
        : max_loot_number_by_map_(handler.max_loot_number_by_map_),
          base_max_loot_number_(handler.base_max_loot_number_) {}

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & max_loot_number_by_map_;
        ar & base_max_loot_number_;
    }

    LootNumberMapHandler::Pointer Restore() const {
        return std::make_shared<LootNumberMapHandler>(max_loot_number_by_map_,
                                                      base_max_loot_number_);
    }

   private:
    LootNumberMapHandler::LootNumberByMap max_loot_number_by_map_;
    int base_max_loot_number_;
};

class ApplicationRepr {
   public:
    ApplicationRepr() = default;

    explicit ApplicationRepr(const app::Application& application)
        : players_(*application.players_),
          game_(*application.game_),
          loot_generator_(*application.loot_generator_),
          loot_handler_(*application.loot_handler_),
          loot_number_map_handler_(*application.loot_number_map_handler_),
          is_random_spawn_point_(application.is_random_spawn_point_) {}

    template <typename Archive>
    void serialize(Archive& archive, [[maybe_unused]] const unsigned version) {
        archive & players_;
        archive & game_;
        archive & loot_generator_;
        archive & loot_handler_;
        archive & loot_number_map_handler_;
        archive & is_random_spawn_point_;
    }

    [[nodiscard]] app::Application::Pointer Restore() const {
        auto game = game_.Restore();
        auto players = players_.Restore([&game](const model::Map::Id map_id) {
            return game->FindGameSession(map_id);
        });
        return std::make_unique<app::Application>(
            players, game, loot_generator_.Restore(), loot_handler_.Restore(),
            loot_number_map_handler_.Restore(), is_random_spawn_point_);
    }

   private:
    serialization::PlayersRepr players_;
    serialization::GameRepr game_;
    serialization::LootGeneratorRepr loot_generator_;
    serialization::LootHandlerRepr loot_handler_;
    serialization::LootNumberMapHandlerRepr loot_number_map_handler_;
    bool is_random_spawn_point_;
};

}  // namespace serialization
