#pragma once

#include <algorithm>
#include <deque>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "game_session.h"
#include "model/model.h"
#include "model/tagged.h"
#include "utils/logger.h"

namespace app {

struct PlayerSession {
    model::Dog::Id dog_id;
    model::Map::Id map_id;

    bool operator==(const PlayerSession& other) const {
        return dog_id == other.dog_id && map_id == other.map_id;
    }
};

class Player {
   public:
    using Id = model::Dog::Id;
    using Position = model::Coordinate;
    using Velocity = model::Coordinate;
    using Direction = model::Direction;

    using Pointer = Player*;
    using ConstPointer = const Player*;
    using GameSessionPointer = GameSession::Pointer;
    explicit Player(GameSessionPointer session, model::Dog* dog)
        : session_(session), dog_(dog) {}

    std::shared_ptr<GameSession> GetSession() const { return session_; }
    Id GetId() const { return dog_->GetId(); }
    model::Dog* GetDog() const { return dog_; }

   private:
    std::shared_ptr<GameSession> session_;
    model::Dog* dog_;
};

namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class Players {
   public:
    using TokenPointer = Token*;
    using ConstTokenPointer = const Token*;
    using GameSessionPointer = GameSession::Pointer;

    Players() = default;

    Players(Players&& players)
        : players_(std::move(players.players_)),
          tokens_(std::move(players.tokens_)),
          session_to_player_(std::move(players.session_to_player_)),
          token_to_player_(std::move(players.token_to_player_)),
          player_to_token_(std::move(players.player_to_token_)) {}

    std::pair<Player::Id, Token> Add(GameSessionPointer session,
                                     model::Dog* dog) {
        auto& player = players_.emplace_back(session, dog);
        session_to_player_.emplace(
            PlayerSession{dog->GetId(), session->GetMap()->GetId()}, &player);

        auto& token = tokens_.emplace_back(GenerateToken());
        token_to_player_.emplace(*token, &player);
        player_to_token_.emplace(dog->GetId(), &token);

        return {player.GetId(), token};
    }

    Player::ConstPointer Find(PlayerSession session) const {
        if (auto it = session_to_player_.find(session);
            it != session_to_player_.end()) {
            return it->second;
        }
        return nullptr;
    }

    Player::Pointer Find(Token token) const {
        if (auto it = token_to_player_.find(*token);
            it != token_to_player_.end()) {
            return it->second;
        }
        return nullptr;
    }

    ConstTokenPointer FindToken(const Player& player) {
        if (auto it = player_to_token_.find(player.GetId());
            it != player_to_token_.end()) {
            return it->second;
        }
        return nullptr;
    }

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
    std::unordered_map<model::Dog::Id, TokenPointer,
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

    std::string GenerateToken() {
        std::string token_str;
        do {
            std::stringstream ss;
            ss << std::hex << std::setw(16) << std::setfill('0')
               << generator1_() << std::setw(16) << std::setfill('0')
               << generator2_();

            token_str = ss.str();
        } while (std::find_if(tokens_.begin(), tokens_.end(),
                              [&token_str](const auto& token) {
                                  return (*token) == token_str;
                              }) != tokens_.end() &&
                 token_str.length() != 32);
        return token_str;
    }
};

}  // namespace app
