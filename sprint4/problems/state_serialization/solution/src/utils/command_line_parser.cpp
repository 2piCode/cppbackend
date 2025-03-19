#include "command_line_parser.h"

#include <stdexcept>

namespace utils {

std::optional<Args> ParseCommandLine(int argc, const char* argv[]) {
    namespace po = boost::program_options;
    using namespace std::literals;

    Args args;
    po::options_description desc("All options");

    int delta_time, save_state_period;
    std::string state_file;
    desc.add_options()("help,h", "produce help message")(
        "tick-period,t",
        po::value(&delta_time)->default_value(0)->value_name("milliseconds"s),
        "set tick period")("config-file,c",
                           po::value(&args.config_file)->value_name("file"s),
                           "set config file path")(
        "www-root,w",
        po::value(&args.static_source_folder)->value_name("path"s),
        "set static files root")(
        "randomize-spawn-points",
        po::bool_switch(&args.is_random_spawnpoint)->default_value(false),
        "spawn dogs at random positions")("state-file", po::value(&state_file),
                                          "set path to app save state")(
        "save-state-period", po::value(&save_state_period),
        "set time period between app state saves");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        BOOST_LOG_TRIVIAL(info) << desc;
        return std::nullopt;
    }

    if (!vm.count("config-file")) {
        throw std::invalid_argument("Config file path is not set"s);
    }

    if (delta_time > 0) {
        args.delta_time = std::chrono::milliseconds(delta_time);
    }

    if (!state_file.empty()) {
        args.state_file = state_file;
    }

    if (save_state_period > 0) {
        args.save_state_period = std::chrono::milliseconds(save_state_period);
    }

    if (!vm.count("www-root")) {
        throw std::invalid_argument("Static files root is not set"s);
    }

    return args;
}

}  // namespace utils
