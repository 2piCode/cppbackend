#include "repository_impl.h"

#include <pqxx/result>
#include <string>
#include <vector>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace postgres {

PlayerRepositoryImpl::PlayerRepositoryImpl(pqxx::work& work) : work_(work) {}

void PlayerRepositoryImpl::Write(const PlayerInfo& player_info) const {
    work_.exec_params(
        "INSERT INTO retired_players (id, name, score, time_in_game) "
        "VALUES ($1, $2, $3, $4);",
        to_string(boost::uuids::random_generator()()), player_info.name,
        player_info.score, player_info.time_in_game);
}

std::vector<PlayerInfo> PlayerRepositoryImpl::Read(int count,
                                                   int max_items) const {
    std::string request =
        "SELECT name, score, time_in_game FROM retired_players ORDER BY score "
        "DESC, "
        "time_in_game, name LIMIT " +
        std::to_string(max_items) + " OFFSET " + std::to_string(count) + ";";
    std::vector<PlayerInfo> players;
    for (const auto [name, score, time_in_game] :
         work_.query<std::string, int, double>(request)) {
        players.emplace_back(name, score, time_in_game);
    }
    return players;
}

}  // namespace postgres
