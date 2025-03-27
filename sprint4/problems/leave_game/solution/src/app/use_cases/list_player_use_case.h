#pragma once

#include <memory>
#include <string>
#include <vector>

#include "app/player/players.h"
#include "app/token.h"
#include "app/use_cases/base.h"

enum class ListPlayerErrorReason { UnknownToken };

struct ListPlayerError : public UseCaseError {
   public:
    ListPlayerError(std::string code, std::string message,
                    ListPlayerErrorReason reason)
        : UseCaseError(code, message), reason(reason) {}

    ListPlayerErrorReason reason;
};

struct PlayerInfo {
    app::Player::Id id;
    std::string_view name;
};

struct ListPlayerResult {
    std::vector<PlayerInfo> player_infos;
};

class ListPlayerUseCase {
   public:
    explicit ListPlayerUseCase(std::shared_ptr<app::PlayersCollection> players)
        : players_(players) {}

    ListPlayerResult GetPlayers(const app::Token& token) const {
        auto player = players_->Find(token);
        if (!player) {
            throw ListPlayerError("unknownToken",
                                  "Player token has not been found",
                                  ListPlayerErrorReason::UnknownToken);
        }

        const auto& dogs = player->GetSession()->GetDogs();
        ListPlayerResult result;
        result.player_infos.reserve(dogs.size());
        for (const auto& dog : dogs) {
            result.player_infos.emplace_back(
                PlayerInfo{dog.GetId(), dog.GetName()});
        }

        return result;
    }

   private:
    const std::shared_ptr<app::PlayersCollection> players_;
};
