#pragma once

#include <filesystem>
#include <memory>

#include "app/game.h"

namespace json_loader {

std::shared_ptr<app::Game> LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader
