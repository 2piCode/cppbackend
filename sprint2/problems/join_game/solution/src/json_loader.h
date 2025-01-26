#pragma once

#include <filesystem>

#include "app/game.h"

namespace json_loader {

app::Game LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader
