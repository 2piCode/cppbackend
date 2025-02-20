#include "api_handler.h"

#include <chrono>
#include <utility>

#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include "app/application.h"
#include "app/token.h"
#include "json_converter.h"
#include "model/model.h"
#include "request_handler/api_handler/parsers/game_tick_request.h"
#include "request_handler/api_handler/parsers/join_game_request.h"
#include "request_handler/api_handler/parsers/player_action_request.h"
#include "request_handler/utils/error_codes.h"
#include "request_handler/utils/response_utils.h"
#include "utils/logger.h"

namespace request_handler::api_handler {

namespace api_keys {

using namespace std::literals;

inline static constexpr beast::string_view ALL_MAPS = "/maps";
inline static constexpr beast::string_view GAME_JOIN = "/game/join";
inline static constexpr beast::string_view GAME_PLAYERS = "/game/players";
inline static constexpr beast::string_view GAME_STATE = "/game/state";
inline static constexpr beast::string_view PLAYER_ACTION =
    "/game/player/action";
inline static constexpr beast::string_view GAME_TICK = "/game/tick";

}  // namespace api_keys

ApiHandler::ApiHandler(app::Application::Pointer app_ptr,
                       bool is_aviable_game_tick)
    : app_ptr_{std::move(app_ptr)},
      is_aviable_game_tick_(is_aviable_game_tick) {
    InitializeRoutes();
}

void ApiHandler::InitializeRoutes() {
    route_map_[api_keys::GAME_JOIN] =
        [this](const http::request<http::string_body>& req) {
            return GameJoin(req.method(), req.body());
        };

    route_map_[api_keys::GAME_PLAYERS] =
        [this](const http::request<http::string_body>& req) {
            return GetGamePlayers(req.method(),
                                  req[http::field::authorization]);
        };

    route_map_[api_keys::ALL_MAPS] =
        [this](const http::request<http::string_body>& req) {
            return GetAllMapsResponse(req.method());
        };

    route_map_[api_keys::GAME_STATE] =
        [this](const http::request<http::string_body>& req) {
            return GetGameState(req.method(), req[http::field::authorization]);
        };

    route_map_[api_keys::PLAYER_ACTION] =
        [this](const http::request<http::string_body>& req) {
            return MovePlayers(req.method(), req[http::field::authorization],
                               req[http::field::content_type], req.body());
        };

    route_map_[api_keys::GAME_TICK] =
        [this](const http::request<http::string_body>& req) {
            return GameTick(req.method(), req[http::field::content_type],
                            req.body());
        };
}

ApiHandler::StringResponse ApiHandler::operator()(
    const http::request<http::string_body>& req) {
    auto target = req.target().substr(API_KEY.size() + API_VERSION_KEY.size());

    auto it = route_map_.find(target);
    if (it != route_map_.end()) {
        return it->second(req);
    }

    if (target.rfind(api_keys::ALL_MAPS) == 0) {
        return GetMapResponse(req.method(), target);
    }

    return response_utils::MakeBadRequestResponse(error_codes::kBadRequest,
                                                  "Bad request");
}

std::optional<app::Token> TryExtractToken(
    const beast::string_view authorization_header) {
    static constexpr beast::string_view BEARER_HEADER_PREFIX = "Bearer ";

    if (authorization_header.empty() ||
        authorization_header.size() !=
            app::token::SIZE + BEARER_HEADER_PREFIX.size() ||
        authorization_header.substr(0, BEARER_HEADER_PREFIX.size()) !=
            BEARER_HEADER_PREFIX) {
        return std::nullopt;
    }

    return app::Token{
        std::string(authorization_header.substr(BEARER_HEADER_PREFIX.size()))};
}

template <typename Fn>
ApiHandler::StringResponse ExecuteAuthorized(
    const beast::string_view authorization_header, Fn&& action) {
    if (auto token = TryExtractToken(authorization_header)) {
        return action(*token);
    } else {
        return response_utils::MakeUnauthorizedResponse(
            error_codes::kInvalidToken, "Invalid token");
    }
}

ApiHandler::StringResponse ApiHandler::GetMapResponse(
    const http::verb method, beast::string_view target) const {
    if (method != http::verb::get) {
        return response_utils::MakeMethodNotAllowedResponse(
            error_codes::kInvalidMethod, http::to_string(http::verb::get));
    }

    auto map_id = model::Map::Id{std::string(
        target.substr(api_keys::ALL_MAPS.size() + 1,
                      target.size() - api_keys::ALL_MAPS.size() - 1))};

    try {
        return response_utils::MakeOkResponse(
            json_converter::FullMapToJson(*app_ptr_->GetMap(map_id)));
    } catch (const GetMapError& error) {
        switch (error.reason) {
            case GetMapErrorReason::MapNotFound:
                return response_utils::MakeNotFoundResponse(error.code,
                                                            error.what());
        }
    }

    return response_utils::MakeBadRequestResponse(error_codes::kBadRequest,
                                                  "Internal server error");
}

ApiHandler::StringResponse ApiHandler::GameJoin(const http::verb method,
                                                const std::string& body) {
    if (method != http::verb::post) {
        return response_utils::MakeMethodNotAllowedResponse(
            error_codes::kInvalidMethod, http::to_string(http::verb::post));
    }

    auto join_game_request = JoinGameRequest::ParseFromJson(body);

    if (!join_game_request) {
        return response_utils::MakeBadRequestResponse(
            error_codes::kInvalidArgument, "Invalid JSON body");
    }

    try {
        auto result =
            app_ptr_->JoinGame(model::Map::Id{join_game_request->map_id},
                               join_game_request->user_name);
        boost::json::object answer{{"authToken", *result.token},
                                   {"playerId", *result.player_id}};
        return response_utils::MakeOkResponse(answer);
    } catch (const JoinGameError& error) {
        switch (error.reason) {
            case JoinGameErrorReason::InvalidMap:
                return response_utils::MakeNotFoundResponse(error.code,
                                                            error.what());
            case JoinGameErrorReason::InvalidName:
                return response_utils::MakeBadRequestResponse(error.code,
                                                              error.what());
        }
    }

    return response_utils::MakeBadRequestResponse(error_codes::kBadRequest,
                                                  "Internal server error");
}

ApiHandler::StringResponse ApiHandler::GetGamePlayers(
    const http::verb method, const beast::string_view authorization_header) {
    if (method != http::verb::get && method != http::verb::head) {
        return response_utils::MakeMethodNotAllowedResponse(
            error_codes::kInvalidMethod, "GET, HEAD");
    }

    return ExecuteAuthorized(
        authorization_header, [&](const app::Token& token) {
            boost::json::array players_json;
            try {
                auto list_player_result = app_ptr_->ListPlayers(token);

                for (const auto& player_info :
                     list_player_result.player_infos) {
                    players_json.push_back(
                        json_converter::PlayerInfoToJson(player_info));
                }
            } catch (const ListPlayerError& error) {
                return response_utils::MakeUnauthorizedResponse(error.code,
                                                                error.what());
            }

            return response_utils::MakeOkResponse(players_json);
        });
}

ApiHandler::StringResponse ApiHandler::GetAllMapsResponse(
    const http::verb method) const {
    if (method != http::verb::get) {
        return response_utils::MakeMethodNotAllowedResponse(
            error_codes::kInvalidMethod, http::to_string(http::verb::get));
    }

    boost::json::array maps;
    for (const auto map : app_ptr_->ListMaps()) {
        maps.push_back(json_converter::MapToJson(map));
    }

    return response_utils::MakeOkResponse(maps);
}

ApiHandler::StringResponse ApiHandler::GetGameState(
    const http::verb method, const beast::string_view authorization_header) {
    if (method != http::verb::get && method != http::verb::head) {
        return response_utils::MakeMethodNotAllowedResponse(
            error_codes::kInvalidMethod, "GET, HEAD");
    }

    return ExecuteAuthorized(
        authorization_header, [this](const app::Token& token) {
            try {
                return response_utils::MakeOkResponse(
                    json_converter::GameStateToJson(
                        app_ptr_->GetGameState(token)));
            } catch (const GetGameStateError& error) {
                return response_utils::MakeUnauthorizedResponse(error.code,
                                                                error.what());
            }
        });
}

ApiHandler::StringResponse ApiHandler::MovePlayers(
    const http::verb method, const beast::string_view authorization_header,
    const beast::string_view content_type, const std::string& body) {
    if (method != http::verb::post) {
        return response_utils::MakeMethodNotAllowedResponse(
            error_codes::kInvalidMethod, http::to_string(http::verb::post));
    }

    if (content_type != "application/json") {
        return response_utils::MakeBadRequestResponse(
            error_codes::kInvalidArgument, "Invalid content type");
    }

    return ExecuteAuthorized(
        authorization_header, [this, &body](const app::Token& token) {
            try {
                auto request = PlayerActionRequest::ParseFromJson(body);

                if (!request) {
                    return response_utils::MakeBadRequestResponse(
                        error_codes::kInvalidArgument,
                        "Failed to parse action");
                }

                app_ptr_->MovePlayer(token, request->direction);
                return response_utils::MakeOkResponse(boost::json::object{});

            } catch (const MovePlayerError& error) {
                return response_utils::MakeUnauthorizedResponse(error.code,
                                                                error.what());
            }
        });
}

ApiHandler::StringResponse ApiHandler::GameTick(
    const http::verb method, const beast::string_view content_type,
    const std::string& body) {
    if (!is_aviable_game_tick_) {
        return response_utils::MakeBadRequestResponse(error_codes::kBadRequest,
                                                      "Invalid endpoint");
    }
    if (method != http::verb::post) {
        return response_utils::MakeMethodNotAllowedResponse(
            error_codes::kInvalidMethod, http::to_string(http::verb::post));
    }

    auto request = GameTickRequest::ParseFromJson(body);
    if (!request) {
        return response_utils::MakeBadRequestResponse(
            error_codes::kInvalidArgument, "Invalid JSON body");
    }
    try {
        app_ptr_->Tick(std::chrono::milliseconds(request->delta_time));
        return response_utils::MakeOkResponse(boost::json::object{});
    } catch (const GameTickError& error) {
        return response_utils::MakeBadRequestResponse(error.code, error.what());
    }
}

}  // namespace request_handler::api_handler
