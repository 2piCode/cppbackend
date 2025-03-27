#pragma once

#include <deque>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>

#include "app/player/player.h"
#include "app/token.h"
#include "model/tagged.h"

namespace serialization {
class PlayersRepr;
}

namespace app {

struct PlayerSession {
    model::Dog::Id dog_id;
    model::Map::Id map_id;

    bool operator==(const PlayerSession& other) const {
        return dog_id == other.dog_id && map_id == other.map_id;
    }
};

class PlayersCollection {
   public:
    virtual ~PlayersCollection() = default;
    virtual std::pair<Player::Id, Token> Add(GameSession::Pointer session,
                                             model::Dog::Pointer dog) {
        throw std::runtime_error(
            "PlayersCollection class not implement method Add");
    }
    virtual void Remove(Player::Id player) {
        throw std::runtime_error(
            "PlayersCollection class not implement method Remove");
    }
    virtual Player::ConstPointer Find(PlayerSession session) const {
        throw std::runtime_error(
            "PlayersCollection class not implement method Find(PlayerSession)");
    }
    virtual Player::Pointer Find(Token token) const {
        throw std::runtime_error(
            "PlayersCollection class not implement method Find(Token)");
    }
    virtual const token::Pointer FindToken(const Player::Id player) const {
        throw std::runtime_error(
            "PlayersCollection class not implement method FindToken");
    }
    virtual std::deque<Player> GetPlayers() {
        throw std::runtime_error(
            "PlayersCollection class not implement method GetPlayers");
    }
};

class Players : public PlayersCollection {
   public:
    friend class serialization::PlayersRepr;

    using Pointer = std::shared_ptr<Players>;
    using GameSessionPointer = GameSession::Pointer;

    Players() = default;
    Players(Players&& players);
    ~Players() = default;

    std::pair<Player::Id, Token> Add(GameSessionPointer session,
                                     model::Dog::Pointer dog) override;

    void Remove(Player::Id player) override;

    Player::ConstPointer Find(PlayerSession session) const override;

    Player::Pointer Find(Token token) const override;

    const token::Pointer FindToken(const Player::Id player) const override;

    std::deque<Player> GetPlayers() override { return players_; }

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
