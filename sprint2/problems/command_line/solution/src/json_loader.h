#pragma once

#include <filesystem>

#include "app/game/game.h"

namespace json_loader {

app::Game::Pointer LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader
