#pragma once

#include <filesystem>

#include "app/game/game.h"
#include "loots/loot_generator.h"
#include "loots/loot_handler.h"
#include "loots/loot_number_map_handler.h"

namespace json_loader {

app::Game::Pointer LoadGame(const std::filesystem::path& json_path);

loot_gen::LootGenerator::Pointer LoadLootGenerator(
    const std::filesystem::path& json_path);

LootHandler::Pointer LoadLootHandler(const std::filesystem::path& json_path);

LootNumberMapHandler::Pointer LoadNumberMapHandler(
    const std::filesystem::path& json_path);

}  // namespace json_loader
