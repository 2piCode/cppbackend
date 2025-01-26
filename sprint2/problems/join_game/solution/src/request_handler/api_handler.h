#pragma once

#include <cstdint>
#include <string>
//
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <system_error>
#include <variant>

#include "app/game.h"
#include "content_type.h"
#include "json_converter.h"
#include "model/model.h"

namespace api_handler {

namespace beast = boost::beast;
namespace http = beast::http;

namespace detail {

using namespace std::literals;

inline constexpr const char* API_KEY = "/api";
inline constexpr const char* API_VERSION_KEY = "/v1";

struct ApiKeys {
    ApiKeys() = delete;
    inline static const std::string GET_MAPS =
        std::string(API_KEY) + API_VERSION_KEY + "/maps";
    inline static const std::string GET_MAP = GET_MAPS + "/";
    inline static const std::string GAME_JOIN =
        std::string(API_KEY) + API_VERSION_KEY + "/game/join";
    inline static const std::string GAME_PLAYERS =
        std::string(API_KEY) + API_VERSION_KEY + "/game/players";
};
}  // namespace detail

class ApiHandler {
   public:
    inline static const std::string API_KEY =
        std::string(detail::API_KEY) + detail::API_VERSION_KEY;

    using MapId = model::Map::Id;
    using ConstMapPointer = app::Game::ConstMapPointer;

    ApiHandler(app::Game& game);

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send) {
        auto target = req.target();

        if (target == detail::ApiKeys::GAME_JOIN) {
            return GameJoin(req.method(), req.body(), send);
        }

        if (target == detail::ApiKeys::GAME_PLAYERS) {
            return GetGamePlayers(req.method(), req[http::field::authorization],
                                  send);
        }

        if (req.method() != http::verb::get) {
            return send(http::status::method_not_allowed, "Invalid method",
                        request_handler::ContentType::TEXT_HTML);
        }

        if (target.starts_with(detail::ApiKeys::GET_MAP)) {
            auto target_str =
                target.substr(detail::ApiKeys::GET_MAP.size(),
                              target.size() - detail::ApiKeys::GET_MAP.size());
            auto [status_code, body] =
                GetMapResponse(model::Map::Id{std::string(target_str)});
            return send(status_code, body, request_handler::ContentType::JSON);
        }

        if (target == detail::ApiKeys::GET_MAPS) {
            return send(http::status::ok, GetMapsResponse(),
                        request_handler::ContentType::JSON);
        }

        return send(http::status::bad_request,
                    GetDefaultResponse("badRequest", "Bad Request"));
    }

   private:
    using MapNotFoundAnswer = std::pair<http::status, std::string>;
    static constexpr boost::string_view NO_CACHE_KEY = "no-cache";

    app::Game& game_;

    std::string GetMapsResponse() const;
    std::pair<http::status, std::string> GetMapResponse(const MapId id) const;
    std::variant<MapNotFoundAnswer, ConstMapPointer> TryFindMap(
        const MapId map_id) const;
    std::string GetDefaultResponse(const std::string key,
                                   const std::string message) const;
    std::string FormatAuthorizationAnswer(const std::string token) const;

    template <typename Body, typename Send>
    void GameJoin(const http::verb method, const Body body, Send&& send) {
        if (method != http::verb::post) {
            return send(http::status::method_not_allowed,
                        GetDefaultResponse("invalidMethod",
                                           "Only POST method is expected"),
                        request_handler::ContentType::JSON, NO_CACHE_KEY,
                        "POST");
        }

        boost::system::error_code ec;
        auto json_body = boost::json::parse(body, ec);
        if (ec) {
            return send(http::status::bad_request,
                        GetDefaultResponse("invalidArgument",
                                           "Join game request parse error"),
                        request_handler::ContentType::JSON, NO_CACHE_KEY);
        }

        const std::string username =
            json_body.at("userName").as_string().c_str();

        if (username.length() == 0) {
            return send(
                http::status::bad_request,
                GetDefaultResponse("invalidArgument", "Invalid user name"),
                request_handler::ContentType::JSON, NO_CACHE_KEY);
        }

        const auto map_id =
            model::Map::Id{json_body.at("mapId").as_string().c_str()};

        auto map_ptr = TryFindMap(map_id);
        if (std::holds_alternative<MapNotFoundAnswer>(map_ptr)) {
            auto not_found_answer = std::get<MapNotFoundAnswer>(map_ptr);
            return send(not_found_answer.first, not_found_answer.second,
                        request_handler::ContentType::JSON, NO_CACHE_KEY);
        }
        auto [player_ptr, token_ptr] =
            game_.AddPlayer(username, std::get<ConstMapPointer>(map_ptr));
        boost::json::object answer{
            {"authToken", **token_ptr},
            {"playerId", *player_ptr->GetDog()->GetId()}};
        return send(http::status::ok, boost::json::serialize(answer),
                    request_handler::ContentType::JSON, "no-cache");
    }

    template <typename Send>
    void GetGamePlayers(const http::verb method,
                        const boost::beast::string_view authorization_header,
                        Send&& send) {
        if (method != http::verb::get && method != http::verb::head) {
            return send(http::status::method_not_allowed,
                        GetDefaultResponse("invalidMethod", "Invalid method"),
                        request_handler::ContentType::JSON, NO_CACHE_KEY,
                        "GET, HEAD");
        }

        if (authorization_header.empty() ||
            authorization_header.substr(0, 6) != "Bearer" ||
            authorization_header.substr(6).length() == 0) {
            return send(http::status::unauthorized,
                        GetDefaultResponse("invalidToken",
                                           "Authorization header is missing"),
                        request_handler::ContentType::JSON, NO_CACHE_KEY);
        }

        const auto token_str = authorization_header.substr(6);

        app::Token token{std::string(token_str)};

        auto player = game_.FindPlayer(token);
        if (!player) {
            return send(http::status::unauthorized,
                        GetDefaultResponse("unknownToken",
                                           "Player token has not been found"),
                        request_handler::ContentType::JSON, NO_CACHE_KEY);
        }

        auto& dogs = player->GetSession()->GetDogs();
        boost::json::array players_json;
        for (const auto& dog : dogs) {
            players_json.push_back(json_converter::DogToJson(dog));
        }

        return send(http::status::ok, boost::json::serialize(players_json),
                    request_handler::ContentType::JSON, NO_CACHE_KEY);
    }
};

}  // namespace api_handler
