#pragma once

#include <chrono>
#include <optional>
#include <string>

#include <boost/program_options.hpp>

#include "utils/logger.h"

namespace utils {

struct Args {
    std::optional<std::chrono::milliseconds> delta_time = std::nullopt;
    std::string config_file;
    std::string static_source_folder;
    bool is_random_spawnpoint;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc,
                                                   const char* argv[]);
}  // namespace utils
