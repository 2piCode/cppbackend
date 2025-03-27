#pragma once

#include <string>
#include <unordered_map>

struct GetGameRecordsRequest {
    int start = 0;
    int max_element = 100;

    static GetGameRecordsRequest ParseFromJson(std::string_view target) {
        GetGameRecordsRequest state;
        auto args = ParseTargetArgs(target);

        if (args.contains("start")) {
            state.start = std::stol(args.at("start"));
        }

        if (args.contains("maxItems")) {
            state.max_element = std::stol(args.at("maxItems"));
        }
        return state;
    }

   private:
    static std::unordered_map<std::string, std::string> ParseTargetArgs(
        std::string_view req_target) {
        std::unordered_map<std::string, std::string> args;

        size_t prev = req_target.find('?') + 1;

        size_t next_amper = req_target.find('&');
        while (next_amper != req_target.npos) {
            std::string arg;
            size_t length = next_amper - prev;
            arg = req_target.substr(prev, length);

            auto parsed_arg = ParseArg(arg);
            args.emplace(std::move(parsed_arg.first),
                         std::move(parsed_arg.second));

            prev = next_amper + 1;
            next_amper = req_target.find('&', prev);
        }

        size_t length = next_amper - prev;
        std::string last_arg = std::string(req_target.substr(prev, length));
        auto parsed_arg = ParseArg(last_arg);
        args.emplace(std::move(parsed_arg.first), std::move(parsed_arg.second));

        return args;
    }

    static std::pair<std::string, std::string> ParseArg(std::string_view arg) {
        size_t equal_sign = arg.find('=');
        std::string key = std::string(arg.substr(0, equal_sign));
        std::string value = std::string(arg.substr(equal_sign + 1, arg.npos));

        return std::make_pair(std::move(key), std::move(value));
    }
};
