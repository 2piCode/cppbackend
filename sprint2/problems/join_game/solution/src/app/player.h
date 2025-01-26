#pragma once

#include <atomic>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "game_session.h"
#include "model/model.h"

namespace app {

struct PlayerSession {
    model::Dog::Id dog_id;
    model::Map::Id map_id;
};

class Player {
   public:
    using GameSessionPointer = GameSession::Pointer;
    explicit Player(GameSessionPointer session, model::Dog* dog)
        : session_(session), dog_(dog) {}

    GameSessionPointer GetSession() const { return session_; }
    model::Dog* GetDog() const { return dog_; }

   private:
    GameSessionPointer session_;
    model::Dog* dog_;
};

namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class Players {
   public:
    using PlayerPointer = Player*;
    using ConstPlayerPointer = const Player*;
    using TokenPointer = Token*;
    using ConstTokenPointer = const Token*;
    using GameSessionPointer = GameSession::Pointer;

    Players() = default;

    Players(Players&& players)
        : players_(std::move(players.players_)),
          tokens_(std::move(players.tokens_)),
          token_to_player_(std::move(players.token_to_player_)),
          player_to_token_(std::move(players.player_to_token_)) {}

    std::pair<PlayerPointer, TokenPointer> Add(GameSessionPointer session,
                                               model::Dog* dog) {
        auto& player = players_.emplace_back(session, dog);
        /*session_to_player_.emplace(*/
        /*    PlayerSession{dog->GetId(), session->GetMapId()}, player);*/
        auto& token = tokens_.emplace_back(GenerateToken());
        token_to_player_.emplace(&token, &player);
        player_to_token_.emplace(&player, &token);

        return {&player, &token};
    }

    ConstPlayerPointer Find(PlayerSession session) const {
        /*if (auto it = session_to_player_.find(session);*/
        /*    it != session_to_player_.end()) {*/
        /*    return it->second;*/
        /*}*/
        return nullptr;
    }

    ConstPlayerPointer Find(Token token) const {
        if (auto it = token_to_player_.find(&token);
            it != token_to_player_.end()) {
            return it->second;
        }
        return nullptr;
    }

    ConstTokenPointer FindToken(const Player& player) {
        if (auto it = player_to_token_.find(&player);
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

    std::vector<Player> players_;
    std::vector<Token> tokens_;
    /*std::unordered_map<PlayerSession, PlayerPointer> session_to_player_;*/
    std::unordered_map<ConstTokenPointer, PlayerPointer> token_to_player_;
    std::unordered_map<ConstPlayerPointer, TokenPointer> player_to_token_;

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
        std::stringstream ss1, ss2;
        ss1 << std::hex << generator1_();
        ss2 << std::hex << generator2_();
        return ss1.str() + ss2.str();
    }
};

}  // namespace app
