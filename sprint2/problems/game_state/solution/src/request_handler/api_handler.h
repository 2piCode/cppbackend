#pragma once

#include <string>
//
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include "app/application.h"
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
    inline static const std::string ALL_MAPS =
        std::string(API_KEY) + API_VERSION_KEY + "/maps";
    inline static const std::string GET_MAP = ALL_MAPS + "/";
    inline static const std::string GAME_JOIN =
        std::string(API_KEY) + API_VERSION_KEY + "/game/join";
    inline static const std::string GAME_PLAYERS =
        std::string(API_KEY) + API_VERSION_KEY + "/game/players";
    inline static const std::string GAME_STATE =
        std::string(API_KEY) + API_VERSION_KEY + "/game/state";
};
}  // namespace detail

class ApiHandler {
   public:
    inline static const std::string API_KEY =
        std::string(detail::API_KEY) + detail::API_VERSION_KEY;

    explicit ApiHandler(app::Game::Pointer game) : app_(game) {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send) {
        auto target = req.target();

        if (target == detail::ApiKeys::GAME_JOIN) {
            return GameJoin(req.method(), req.body(), send);
        } else if (target == detail::ApiKeys::GAME_PLAYERS) {
            return GetGamePlayers(req.method(), req[http::field::authorization],
                                  send);
        } else if (target.starts_with(detail::ApiKeys::GET_MAP)) {
            return GetMapResponse(req.method(), target, send);
        } else if (target == detail::ApiKeys::ALL_MAPS) {
            return GetAllMapsResponse(req.method(), send);
        } else if (target == detail::ApiKeys::GAME_STATE) {
            return GetGameState(req.method(), req[http::field::authorization],
                                send);
        }

        return send(http::status::bad_request,
                    GetDefaultResponse("badRequest", "Bad Request"));
    }

   private:
    static constexpr boost::string_view NO_CACHE_KEY = "no-cache";
    static constexpr boost::string_view USERNAME_KEY = "userName";
    static constexpr boost::string_view MAP_ID_KEY = "mapId";
    static constexpr boost::beast::string_view BEARER_HEADER_PREFIX = "Bearer ";
    inline static const std::string INVALID_METHOD_CODE = "invalidMethod";

    app::Application app_;

    std::string GetDefaultResponse(const std::string code,
                                   const std::string message) const {
        return boost::json::serialize(
            boost::json::value{{"code", code}, {"message", message}});
    }

    template <typename Send>
    std::optional<app::Token> CheckAuthorization(
        const boost::beast::string_view authorization_header, Send&& send) {
        if (authorization_header.empty() ||
            authorization_header.size() != 32 + BEARER_HEADER_PREFIX.size() ||
            authorization_header.substr(0, BEARER_HEADER_PREFIX.size()) !=
                BEARER_HEADER_PREFIX) {
            send(http::status::unauthorized,
                 GetDefaultResponse("invalidToken", "Invalid token"),
                 request_handler::ContentType::JSON, NO_CACHE_KEY);
            return std::nullopt;
        }

        return app::Token{std::string(
            authorization_header.substr(BEARER_HEADER_PREFIX.size()))};
    }

    template <typename Body, typename Send>
    void GameJoin(const http::verb method, const Body body, Send&& send) {
        if (method != http::verb::post) {
            return send(http::status::method_not_allowed,
                        GetDefaultResponse(INVALID_METHOD_CODE,
                                           "Only POST method is expected"),
                        request_handler::ContentType::JSON, NO_CACHE_KEY,
                        "POST");
        }

        boost::system::error_code ec;
        const boost::json::value json_body_value = boost::json::parse(body, ec);

        if (ec) {
            return send(http::status::bad_request,
                        GetDefaultResponse(INVALID_METHOD_CODE,
                                           "Join game request parse error"),
                        request_handler::ContentType::JSON, NO_CACHE_KEY);
        }

        const boost::json::object json_body = json_body_value.as_object();

        if (!json_body.contains(USERNAME_KEY) ||
            !json_body.contains(MAP_ID_KEY)) {
            return send(
                http::status::bad_request,
                GetDefaultResponse("invalidArgument", "Invalid JSON body"),
                request_handler::ContentType::JSON, NO_CACHE_KEY);
        }

        try {
            auto result = app_.JoinGame(
                model::Map::Id{json_body.at(MAP_ID_KEY).as_string().c_str()},
                json_body.at(USERNAME_KEY).as_string().c_str());

            boost::json::object answer{{"authToken", *result.token},
                                       {"playerId", *result.player_id}};
            return send(http::status::ok, boost::json::serialize(answer),
                        request_handler::ContentType::JSON, NO_CACHE_KEY);

        } catch (const JoinGameError& error) {
            switch (error.reason) {
                case JoinGameErrorReason::InvalidName:
                    return send(http::status::bad_request,
                                GetDefaultResponse(error.code, error.what()),
                                request_handler::ContentType::JSON,
                                NO_CACHE_KEY);

                case JoinGameErrorReason::InvalidMap:
                    return send(http::status::not_found,
                                GetDefaultResponse(error.code, error.what()),
                                request_handler::ContentType::JSON,
                                NO_CACHE_KEY);
            }
        }
    }

    template <typename Send>
    void GetGamePlayers(const http::verb method,
                        const boost::beast::string_view authorization_header,
                        Send&& send) {
        if (method != http::verb::get && method != http::verb::head) {
            return send(
                http::status::method_not_allowed,
                GetDefaultResponse(INVALID_METHOD_CODE, "Invalid method"),
                request_handler::ContentType::JSON, NO_CACHE_KEY, "GET, HEAD");
        }

        auto token = CheckAuthorization(authorization_header, send);
        if (!token) {
            return;
        }

        boost::json::array players_json;
        try {
            auto list_player_result = app_.ListPlayers(*token);

            for (const auto& player_info : list_player_result.player_infos) {
                players_json.push_back(
                    json_converter::PlayerInfoToJson(player_info));
            }
        } catch (const ListPlayerError& error) {
            auto response = GetDefaultResponse(error.code, error.what());
            switch (error.reason) {
                case ListPlayerErrorReason::InvalidToken:

                case ListPlayerErrorReason::UnknownToken:
                    return send(http::status::unauthorized, response,
                                request_handler::ContentType::JSON,
                                NO_CACHE_KEY);
            }
        }

        return send(http::status::ok, boost::json::serialize(players_json),
                    request_handler::ContentType::JSON, NO_CACHE_KEY);
    }

    template <typename Send>
    void GetMapResponse(const http::verb method,
                        boost::beast::string_view target, Send&& send) const {
        if (method != http::verb::get) {
            return send(http::status::method_not_allowed, "Invalid method",
                        request_handler::ContentType::TEXT_HTML);
        }

        auto map_id = model::Map::Id{std::string(
            target.substr(detail::ApiKeys::GET_MAP.size(),
                          target.size() - detail::ApiKeys::GET_MAP.size()))};

        try {
            auto map = app_.GetMap(map_id);
            return send(
                http::status::ok,
                boost::json::serialize(json_converter::FullMapToJson(*map)),
                request_handler::ContentType::JSON);
        } catch (const GetMapError& error) {
            switch (error.reason) {
                case GetMapErrorReason::MapNotFound:
                    return send(http::status::not_found,
                                GetDefaultResponse(error.code, error.what()),
                                request_handler::ContentType::JSON,
                                NO_CACHE_KEY);
            }
        }
    }

    template <typename Send>
    void GetAllMapsResponse(const http::verb method, Send&& send) const {
        if (method != http::verb::get) {
            return send(http::status::method_not_allowed, "Invalid method",
                        request_handler::ContentType::TEXT_HTML);
        }

        boost::json::array maps;
        for (const auto map : app_.ListMaps()) {
            maps.push_back(json_converter::MapToJson(map));
        }

        return send(http::status::ok, boost::json::serialize(maps),
                    request_handler::ContentType::JSON);
    }

    template <typename Send>
    void GetGameState(const http::verb method,
                      const boost::beast::string_view authorization_header,
                      Send&& send) {
        if (method != http::verb::get && method != http::verb::head) {
            return send(
                http::status::method_not_allowed,
                GetDefaultResponse(INVALID_METHOD_CODE, "Invalid method"),
                request_handler::ContentType::JSON, NO_CACHE_KEY, "GET, HEAD");
        }

        auto token = CheckAuthorization(authorization_header, send);
        if (!token) {
            return;
        }

        try {
            auto game_state = app_.GetGameState(*token);
            return send(http::status::ok,
                        boost::json::serialize(
                            json_converter::GameStateToJson(game_state)),
                        request_handler::ContentType::JSON, NO_CACHE_KEY);
        } catch (const GetGameStateError& error) {
            switch (error.reason) {
                case GetGameStateErrorReason::UnknownToken:
                    return send(http::status::unauthorized,
                                GetDefaultResponse(error.code, error.what()),
                                request_handler::ContentType::JSON,
                                NO_CACHE_KEY);
            }
        }
    }
};

}  // namespace api_handler
