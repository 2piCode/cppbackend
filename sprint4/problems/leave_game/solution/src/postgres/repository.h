#pragma once

#include <string>
#include <vector>

namespace postgres {

struct PlayerInfo {
    std::string name;
    int score;
    double time_in_game;
};

class PlayerRepository {
   public:
    virtual ~PlayerRepository() = default;

    virtual void Write(const PlayerInfo& player) const = 0;
    virtual std::vector<PlayerInfo> Read(int count, int max_items) const = 0;
};
}  // namespace postgres
