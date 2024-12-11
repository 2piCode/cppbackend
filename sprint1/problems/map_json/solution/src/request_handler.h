#pragma once
#include <cinttypes>
#include <optional>
#include <typeinfo>
#include <utility>

#include "boost/beast/http/status.hpp"
#include "boost/json/serialize.hpp"
#include "http_server.h"
#include "json_converter.h"
#include "model.h"

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;

namespace {

using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view JSON = "application/json"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы
    // контента
};

struct ContentPath {
    ContentPath() = delete;
    constexpr static std::string_view GET_MAPS = "/maps";
};

}  // namespace

class RequestHandler {
   public:
    using JsonResponse = http::response<http::string_body>;

    explicit RequestHandler(model::Game& game) : game_{game} {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
                    Send&& send) {
        using namespace std::literals;
        // Обработать запрос request и отправить ответ, используя send
        const auto text_response =
            [this, &req, &send](http::status status, std::string_view answer) {
                return send(MakeStringResponse(status, answer, req.version(),
                                               req.keep_alive()));
            };

        if (req.method() == http::verb::get) {
            if (req.target() == ContentPath::GET_MAPS) {
                return text_response(http::status::ok, GetMapsResponse());
            }

            auto start_it = req.target().find("/maps/"sv);

            if (start_it != std::string::npos) {
                auto target_str =
                    req.target().substr(ContentPath::GET_MAPS.size() + 1);
                auto [status_code, body] = GetMapResponse(target_str);
                return text_response(status_code, body);
            }
        }

        return text_response(http::status::bad_request, GetBadResponse());
    }

   private:
    model::Game& game_;

    JsonResponse MakeStringResponse(
        http::status status, std::string_view body, unsigned http_version,
        bool keep_alive,
        std::string_view content_type = ContentType::TEXT_HTML) {
        JsonResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    std::string GetMapsResponse() {
        boost::json::object main;
        boost::json::array maps;
        for (const auto map : game_.GetMaps()) {
            maps.push_back(json_converter::MapToJson(map));
        }
        main["maps"] = maps;
        return boost::json::serialize(main);
    }

    std::pair<http::status, std::string> GetMapResponse(std::string id) {
        auto map = game_.FindMap(model::Map::Id{id});
        if (!map) {
            return std::make_pair(http::status::not_found,
                                  boost::json::serialize(GetDefaultResponse(
                                      "mapNotFound", "Map not found")));
        }

        auto map_json = json_converter::MapToJson(*map);
        return std::make_pair(http::status::ok,
                              boost::json::serialize(map_json));
    }

    std::string GetBadResponse() {
        return boost::json::serialize(
            GetDefaultResponse("badRequest", "Bad Request"));
    }

    boost::json::object GetDefaultResponse(std::string code,
                                           std::string message) {
        boost::json::object main;
        main["code"] = code;
        main["message"] = message;
        return main;
    }
};

}  // namespace http_handler
