#pragma once

#include <filesystem>
#include <optional>

#include "app/application.h"

void SaveApplicationState(app::Application::Pointer app_ptr,
                          std::filesystem::path state_file);

std::optional<app::Application::Pointer> LoadApplicationState(
    std::filesystem::path state_file);
