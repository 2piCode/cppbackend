#pragma once

#include <optional>
#include <string>
//
#include <boost/json.hpp>

#include "model/model.h"

namespace request_handler::api_handler {

struct PlayerActionRequest {
    model::Direction direction;

    static std::optional<PlayerActionRequest> ParseFromJson(
        const std::string& body) {
        static constexpr boost::json::string_view direction_key = "move";

        boost::system::error_code ec;
        const boost::json::value json_body_value = boost::json::parse(body, ec);

        if (ec) {
            return std::nullopt;
        }

        const boost::json::object json_body = json_body_value.as_object();

        auto direction_it = json_body.find(direction_key);
        if (direction_it == json_body.end()) {
            return std::nullopt;
        }

        return PlayerActionRequest{
            .direction = model::direction_converter::ToDirection(
                std::string(direction_it->value().as_string()))};
    }
};

}  // namespace request_handler::api_handler
