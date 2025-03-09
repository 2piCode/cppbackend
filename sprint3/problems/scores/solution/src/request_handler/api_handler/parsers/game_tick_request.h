#pragma once

#include <string>

#include "parser.h"

struct GameTickRequest {
    int delta_time;

    static std::optional<GameTickRequest> ParseFromJson(
        const std::string& body) {
        static constexpr boost::json::string_view delta_time_key = "timeDelta";

        const auto json_body = Parse(body);
        if (!json_body) {
            return std::nullopt;
        }

        auto delta_time_it = json_body->find(delta_time_key);
        if (delta_time_it == json_body->end()) {
            return std::nullopt;
        }

        if (!delta_time_it->value().is_int64()) {
            return std::nullopt;
        }

        return GameTickRequest{
            .delta_time = static_cast<int>(delta_time_it->value().as_int64())};
    }
};
