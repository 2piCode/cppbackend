#pragma once

#include <optional>
#include <string>

#include "model/model.h"
#include "parser.h"

namespace request_handler::api_handler {

struct PlayerActionRequest {
    model::Direction direction;

    static std::optional<PlayerActionRequest> ParseFromJson(
        const std::string& body) {
        static constexpr boost::json::string_view direction_key = "move";

        const auto json_body = Parse(body);
        if (!json_body) {
            return std::nullopt;
        }

        auto direction_it = json_body->find(direction_key);
        if (direction_it == json_body->end()) {
            return std::nullopt;
        }

        return PlayerActionRequest{
            .direction = model::direction_converter::ToDirection(
                std::string(direction_it->value().as_string()))};
    }
};

}  // namespace request_handler::api_handler
