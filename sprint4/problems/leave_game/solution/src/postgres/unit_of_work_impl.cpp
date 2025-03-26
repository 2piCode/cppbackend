#include "unit_of_work_impl.h"

#include <algorithm>
#include <utility>

#include "postgres/connection_pool.h"
#include "utils/logger.h"

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

Database::Database(pqxx::work& work) : work_(work) {
    work_.exec(R"(
CREATE TABLE IF NOT EXISTS retired_players (
   id UUID CONSTRAINT retired_players_pkey PRIMARY KEY,
   name varchar(100) NOT NULL,
   score integer NOT NULL,
   time_in_game real NOT NULL
);
)"_zv);

    work_.exec(R"(
CREATE INDEX IF NOT EXISTS idx_score_playtime_name
ON retired_players (score DESC, time_in_game, name);
)"_zv);
}

PlayerRepositoryImpl& Database::GetPlayers() { return players_; }

UnitOfWorkImpl::UnitOfWorkImpl(ConnectionPool::ConnectionWrapper wrapper)
    : wrapper_(std::move(wrapper)), db_(work_) {}

void UnitOfWorkImpl::Commit() { work_.commit(); }

PlayerRepository& UnitOfWorkImpl::GetPlayers() { return db_.GetPlayers(); }

UnitOfWorkFactoryImpl::UnitOfWorkFactoryImpl(
    std::unique_ptr<ConnectionPool> pool)
    : pool_(std::move(pool)) {}

std::unique_ptr<UnitOfWork> UnitOfWorkFactoryImpl::CreateUnitOfWork() {
    return std::make_unique<postgres::UnitOfWorkImpl>(pool_->GetConnection());
}

std::shared_ptr<UnitOfWorkFactory> CreateFactory() {
    if (const auto* url = std::getenv("GAME_DB_URL")) {
        static const std::string db_url = url;
        return std::make_shared<UnitOfWorkFactoryImpl>(
            std::make_unique<ConnectionPool>(
                std::max(1u, std::thread::hardware_concurrency()),
                [] { return std::make_shared<pqxx::connection>(db_url); }));
    }
    throw std::runtime_error("Missing GAME_DB_URL environment variable");
}

}  // namespace postgres
