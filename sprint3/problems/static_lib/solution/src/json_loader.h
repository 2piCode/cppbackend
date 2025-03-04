#pragma once

#include <filesystem>

#include "app/game/game.h"
#include "loot_generator.h"
#include "loot_handler.h"

namespace json_loader {

app::Game::Pointer LoadGame(const std::filesystem::path& json_path);

loot_gen::LootGenerator::Pointer LoadLootGenerator(
    const std::filesystem::path& json_path);

LootHandler::Pointer LoadLootHandler(const std::filesystem::path& json_path);

}  // namespace json_loader
