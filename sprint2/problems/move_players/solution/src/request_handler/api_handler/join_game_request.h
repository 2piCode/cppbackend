#pragma once

#include <optional>
#include <string>
//
#include <boost/json.hpp>

namespace request_handler::api_handler {

struct JoinGameRequest {
    std::string map_id;
    std::string user_name;

    static std::optional<JoinGameRequest> ParseFromJson(
        const std::string& body) {
        static constexpr boost::json::string_view map_id_key = "mapId";
        static constexpr boost::json::string_view user_name_key = "userName";

        boost::system::error_code ec;
        const boost::json::value json_body_value = boost::json::parse(body, ec);

        if (ec) {
            return std::nullopt;
        }

        const boost::json::object json_body = json_body_value.as_object();

        auto map_id_it = json_body.find(map_id_key),
             user_name_it = json_body.find(user_name_key);
        if (map_id_it == json_body.end() || user_name_it == json_body.end()) {
            return std::nullopt;
        }

        return JoinGameRequest{
            .map_id = std::string(map_id_it->value().as_string()),
            .user_name = std::string(user_name_it->value().as_string())};
    }
};

}  // namespace request_handler::api_handler
