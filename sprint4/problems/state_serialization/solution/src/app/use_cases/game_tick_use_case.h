#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <boost/log/trivial.hpp>

#include "app/collision_detector.h"
#include "app/game/game.h"
#include "app/player/players.h"
#include "app/spawn_point_generator.h"
#include "base.h"
#include "loots/loot_generator.h"
#include "loots/loot_handler.h"
#include "loots/loot_number_map_handler.h"
#include "model/item.h"
#include "model/tagged.h"
#include "utils/logger.h"

enum class GameTickErrorReason { InvalidDeltaTime };

struct GameTickError : public UseCaseError {
    GameTickError(std::string code, std::string message,
                  GameTickErrorReason reason)
        : UseCaseError(code, message), reason_(reason) {}
    GameTickErrorReason reason_;
};

class ItemGatherer : public collision_detector::ItemGathererProvider {
   public:
    using Items = std::vector<collision_detector::Item>;
    using Gatherers = std::vector<collision_detector::Gatherer>;

    ItemGatherer(const Items& items, const Gatherers& gatherers)
        : items_(items), gatherers_(gatherers) {}

    size_t ItemsCount() const override { return items_.size(); }
    collision_detector::Item GetItem(size_t idx) const override {
        return items_.at(idx);
    }
    size_t GatherersCount() const override { return gatherers_.size(); }
    collision_detector::Gatherer GetGatherer(size_t idx) const override {
        return gatherers_.at(idx);
    }

   private:
    const Items& items_;
    const Gatherers& gatherers_;
};

class GameTickUseCase {
   public:
    explicit GameTickUseCase(
        app::Game::Pointer game,
        std::shared_ptr<app::PlayersCollection> players,
        loot_gen::LootGenerator::Pointer loot_generator,
        LootHandler::Pointer loot_handler,
        LootNumberMapHandler::Pointer loot_number_map_handler,
        bool is_random_spawn_point = true)
        : game_(game),
          players_(players),
          loot_generator_(std::move(loot_generator)),
          loot_handler_(loot_handler),
          loot_number_map_handler_(std::move(loot_number_map_handler)),
          spawn_point_generator_(is_random_spawn_point) {}

    void Tick(std::chrono::milliseconds delta_time) {
        if (delta_time.count() <= 0) {
            throw GameTickError("invalidArgument", "Invalid delta time",
                                GameTickErrorReason::InvalidDeltaTime);
        }
        MovePlayers(delta_time);
        GenerateLoot(delta_time);
    }

   private:
    enum CollisionItemType { Item, Office };

    app::Game::Pointer game_;
    std::shared_ptr<app::PlayersCollection> players_;
    loot_gen::LootGenerator::Pointer loot_generator_;
    LootHandler::Pointer loot_handler_;

    LootNumberMapHandler::Pointer loot_number_map_handler_;
    SpawnPointGenerator spawn_point_generator_;

    std::random_device random_device_;
    std::mt19937_64 generator_{random_device_()};

    void MovePlayers(std::chrono::milliseconds delta_time) {
        std::unordered_map<model::Map::Id, ItemGatherer::Gatherers,
                           util::TaggedHasher<model::Map::Id>>
            map_gatherers;

        std::unordered_map<model::Map::Id, ItemGatherer::Items,
                           util::TaggedHasher<model::Map::Id>>
            map_items;

        std::unordered_map<size_t, app::Player::Pointer> gatherer_id_player;
        for (auto& player : players_->GetPlayers()) {
            auto [start_pos, end_pos] = player.Move(delta_time);
            auto map_id = player.GetSession()->GetMapId();
            map_gatherers[map_id].push_back(
                collision_detector::Gatherer{.start_pos = start_pos,
                                             .end_pos = end_pos,
                                             .width = app::Player::WIDTH / 2});
            gatherer_id_player.emplace(map_gatherers[map_id].size(), &player);
        }

        std::unordered_map<size_t,
                           std::variant<model::Item::Id, const model::Office*>>
            item_id_item;

        for (const auto& [map, _] : map_gatherers) {
            auto game_session = game_->FindGameSession(map);

            for (const auto& item : game_session->GetLootPositionsInfo()) {
                map_items[map].push_back(collision_detector::Item{
                    .position = item.position, .width = model::ItemWidth / 2});
                item_id_item.emplace(map_items[map].size(), item.id);
            }

            for (const auto& office : game_session->GetMap()->GetOffices()) {
                map_items[map].push_back(collision_detector::Item{
                    .position =
                        model::Coordinate{
                            .x = static_cast<double>(office.GetPosition().x),
                            .y = static_cast<double>(office.GetPosition().y)},
                    .width = model::Office::WIDTH / 2});
                item_id_item.emplace(map_items[map].size(), &office);
            }
        }

        for (const auto& [map, gatherers] : map_gatherers) {
            const auto& items = map_items[map];
            ItemGatherer provider(items, gatherers);
            std::vector<collision_detector::GatheringEvent> events =
                collision_detector::FindGatherEvents(provider);
            ProcessEvents(gatherer_id_player, item_id_item, events);
        }
    }

    void ProcessEvents(
        const std::unordered_map<size_t, app::Player::Pointer>&
            gatherer_id_gatherer,
        const std::unordered_map<
            size_t, std::variant<model::Item::Id, const model::Office*>>&
            item_id_item,
        const std::vector<collision_detector::GatheringEvent>& events) {
        for (const auto& event : events) {
            auto player = gatherer_id_gatherer.at(event.gatherer_id);
            auto item = item_id_item.at(event.item_id);
            if (auto item_ptr = std::get_if<const model::Office*>(&item)) {
                player->DropAllItems();
            } else if (auto item_ptr = std::get_if<model::Item::Id>(&item)) {
                if (auto removed_item =
                        player->GetSession()->RemoveLoot(*item_ptr);
                    removed_item.has_value()) {
                    player->AddItem(*removed_item);
                }
            }
        }
    }

    int GenerateType(int max_type) {
        std::uniform_int_distribution<std::int32_t> distribution(1, max_type);
        return distribution(generator_);
    }

    void GenerateLoot(std::chrono::milliseconds delta_time) {
        for (const auto& map : game_->GetMaps()) {
            auto session = game_->FindGameSession(map->GetId());
            if (!session) {
                continue;
            }
            int count_items =
                loot_generator_->Generate(delta_time, session->GetLootNumber(),
                                          session->GetDogs().size());
            for (size_t i = 0; i < count_items; i++) {
                auto spawn_point = spawn_point_generator_.Generate(map);
                int type = GenerateType(map->GetNumberOfLootTypes());
                session->AddLoot(
                    type, spawn_point,
                    loot_handler_->FindValueByLootType(map->GetId(), type));
            }
        }
    }
};
