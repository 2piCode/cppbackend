#pragma once

#include <deque>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>

#include "app/player/player.h"
#include "app/token.h"
#include "model/tagged.h"

namespace app {

struct PlayerSession {
    model::Dog::Id dog_id;
    model::Map::Id map_id;

    bool operator==(const PlayerSession& other) const {
        return dog_id == other.dog_id && map_id == other.map_id;
    }
};

class Players {
   public:
    using GameSessionPointer = GameSession::Pointer;

    Players() = default;
    Players(Players&& players);

    std::pair<Player::Id, Token> Add(GameSessionPointer session,
                                     model::Dog* dog);

    Player::ConstPointer Find(PlayerSession session) const;

    Player::Pointer Find(Token token) const;

    const token::Pointer FindToken(const Player& player) const;

   private:
    struct PlayerSessionComparator {
        std::size_t operator()(const PlayerSession& session) const {
            return std::hash<std::uint32_t>{}(*session.dog_id) +
                   std::hash<std::string>{}(*session.map_id);
        }
    };

    std::deque<Player> players_;
    std::deque<Token> tokens_;
    std::unordered_map<PlayerSession, Player::Pointer, PlayerSessionComparator>
        session_to_player_;
    std::unordered_map<std::string_view, Player::Pointer> token_to_player_;
    std::unordered_map<model::Dog::Id, token::Pointer,
                       util::TaggedHasher<model::Dog::Id>>
        player_to_token_;

    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};

    std::string GenerateToken();
};

}  // namespace app
