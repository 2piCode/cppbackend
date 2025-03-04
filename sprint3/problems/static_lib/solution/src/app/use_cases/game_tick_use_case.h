#pragma once

#include <memory>
#include <string>
#include <utility>

#include <boost/log/trivial.hpp>

#include "app/game/game.h"
#include "app/player/players.h"
#include "app/spawn_point_generator.h"
#include "base.h"
#include "loot_generator.h"
#include "utils/logger.h"

enum class GameTickErrorReason { InvalidDeltaTime };

struct GameTickError : public UseCaseError {
    GameTickError(std::string code, std::string message,
                  GameTickErrorReason reason)
        : UseCaseError(code, message), reason_(reason) {}
    GameTickErrorReason reason_;
};

class GameTickUseCase {
   public:
    explicit GameTickUseCase(app::Game::Pointer game,
                             std::shared_ptr<app::PlayersCollection> players,
                             loot_gen::LootGenerator::Pointer loot_generator,
                             bool is_random_spawn_point = true)
        : game_(game),
          players_(players),
          loot_generator_(std::move(loot_generator)),
          spawn_point_generator_(is_random_spawn_point) {}

    void Tick(std::chrono::milliseconds delta_time) {
        if (delta_time.count() <= 0) {
            throw GameTickError("invalidArgument", "Invalid delta time",
                                GameTickErrorReason::InvalidDeltaTime);
        }
        players_->Tick(delta_time);

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
                session->AddLoot(GenerateType(map->GetNumberOfLootTypes()),
                                 spawn_point);
            }
        }
    }

   private:
    app::Game::Pointer game_;
    std::shared_ptr<app::PlayersCollection> players_;
    loot_gen::LootGenerator::Pointer loot_generator_;
    SpawnPointGenerator spawn_point_generator_;

    std::random_device random_device_;
    std::mt19937_64 generator_{random_device_()};

    int GenerateType(int max_type) {
        std::uniform_int_distribution<std::int32_t> distribution(1, max_type);
        return distribution(generator_);
    }
};
