#include "players.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

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
    player_to_token_.emplace(dog->GetId(), &token);

    return {player.GetId(), token};
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

const token::Pointer Players::FindToken(const Player& player) const {
    if (auto it = player_to_token_.find(player.GetId());
        it != player_to_token_.end()) {
        return it->second;
    }
    return nullptr;
}

void Players::Tick(std::chrono::milliseconds delta_time) {
    for (auto& player : players_) {
        player.Tick(delta_time);
    }
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
