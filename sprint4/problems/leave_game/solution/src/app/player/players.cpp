#include "players.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

#include "app/token.h"

namespace app {

Players::Players(Players&& players)
    : players_(std::move(players.players_)),
      tokens_(std::move(players.tokens_)),
      session_to_player_(std::move(players.session_to_player_)),
      token_to_player_(std::move(players.token_to_player_)),
      player_to_token_(std::move(players.player_to_token_)) {}

std::pair<Player::Id, Token> Players::Add(GameSessionPointer session,
                                          model::Dog::Pointer dog) {
    auto& player = players_.emplace_back(session, dog);
    session_to_player_.emplace(
        PlayerSession{dog->GetId(), session->GetMap()->GetId()}, &player);

    auto& token = tokens_.emplace_back(GenerateToken());
    token_to_player_.emplace(*token, &player);
    player_to_token_.emplace(player.GetId(), &token);
    return {player.GetId(), token};
}

void Players::Remove(Player::Id player_id) {
    if (auto player_it = std::find_if(
            players_.begin(), players_.end(),
            [&player_id](auto& player) { return player.GetId() == player_id; });
        player_it != players_.end()) {
        app::Player& player = *player_it;
        session_to_player_.erase(
            PlayerSession{player_id, player.GetSession()->GetMap()->GetId()});
        token_to_player_.erase(**player_to_token_.at(player_id));
        player_to_token_.erase(player_id);
        players_.erase(player_it);
    }
}

Player::ConstPointer Players::Find(PlayerSession session) const {
    if (auto it = session_to_player_.find(session);
        it != session_to_player_.end()) {
        return it->second;
    }
    return nullptr;
}

Player::Pointer Players::Find(Token token) const {
    if (auto it = token_to_player_.find(*token); it != token_to_player_.end()) {
        return it->second;
    }
    return nullptr;
}

const token::Pointer Players::FindToken(const Player::Id player_id) const {
    if (auto it = player_to_token_.find(player_id);
        it != player_to_token_.end()) {
        return it->second;
    }
    return nullptr;
}

std::string Players::GenerateToken() {
    std::string token_str;
    do {
        std::stringstream ss;
        ss << std::hex << std::setw(token::SIZE / 2) << std::setfill('0')
           << generator1_() << std::setw(token::SIZE / 2) << std::setfill('0')
           << generator2_();

        token_str = ss.str();
    } while (std::find_if(tokens_.begin(), tokens_.end(),
                          [&token_str](const auto& token) {
                              return (*token) == token_str;
                          }) != tokens_.end() &&
             token_str.length() != token::SIZE);
    return token_str;
}

}  // namespace app
