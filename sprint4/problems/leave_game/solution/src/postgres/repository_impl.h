#pragma once

#include <pqxx/transaction>

#include "repository.h"

namespace postgres {

class PlayerRepositoryImpl : public PlayerRepository {
   public:
    PlayerRepositoryImpl(pqxx::work& work);

    void Write(const PlayerInfo& player_info) const override;
    std::vector<PlayerInfo> Read(int count, int max_items) const override;

   private:
    pqxx::work& work_;
};
}  // namespace postgres
