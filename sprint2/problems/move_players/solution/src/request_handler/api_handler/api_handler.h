
#pragma once

#include <string>
#include <unordered_map>
//
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>

#include "app/application.h"
#include "app/game/game.h"
#include "request_handler/utils/response_utils.h"

namespace request_handler::api_handler {

namespace beast = boost::beast;
namespace http = beast::http;

class ApiHandler {
   public:
    using StringResponse = response_utils::StringResponse;

    static constexpr beast::string_view API_KEY = "/api";
    static constexpr beast::string_view API_VERSION_KEY = "/v1";

    explicit ApiHandler(app::Game::Pointer game);

    StringResponse operator()(const http::request<http::string_body>& req);

   private:
    struct BeastStringViewHasher {
        std::size_t operator()(const beast::string_view& sv) const noexcept {
            return std::hash<std::string_view>{}(
                std::string_view(sv.data(), sv.size()));
        }
    };

    struct BeastStringViewEqual {
        bool operator()(const beast::string_view& lhs,
                        const beast::string_view& rhs) const noexcept {
            return lhs == rhs;
        }
    };

    using Routes = std::unordered_map<
        beast::string_view,
        std::function<StringResponse(const http::request<http::string_body>&)>,
        BeastStringViewHasher, BeastStringViewEqual>;

    app::Application app_;
    Routes route_map_;

    void InitializeRoutes();

    StringResponse GetMapResponse(const http::verb method,
                                  beast::string_view target) const;

    StringResponse GameJoin(const http::verb method, const std::string& body);

    StringResponse GetGamePlayers(
        const http::verb method, const beast::string_view authorization_header);

    StringResponse GetAllMapsResponse(const http::verb method) const;

    StringResponse GetGameState(const http::verb method,
                                const beast::string_view authorization_header);

    StringResponse MovePlayers(const http::verb method,
                               const beast::string_view authorization_headet,
                               const beast::string_view content_type,
                               const std::string& body);
};

}  // namespace request_handler::api_handler
