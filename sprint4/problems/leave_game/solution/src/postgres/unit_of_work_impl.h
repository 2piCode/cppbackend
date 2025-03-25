#pragma once

#include <memory>
#include <pqxx/connection>
#include <pqxx/transaction>

#include "connection_pool.h"
#include "postgres/repository.h"
#include "postgres/repository_impl.h"
#include "unit_of_work.h"

namespace postgres {

class Database {
   public:
    explicit Database(pqxx::work& work);

    PlayerRepositoryImpl& GetPlayers();

   private:
    pqxx::work& work_;
    PlayerRepositoryImpl players_{work_};
};

class UnitOfWorkImpl : public UnitOfWork {
   public:
    explicit UnitOfWorkImpl(ConnectionPool::ConnectionWrapper wrapper);

    ~UnitOfWorkImpl() override;

    void Commit() override;
    PlayerRepository& GetPlayers() override;

   private:
    ConnectionPool::ConnectionWrapper wrapper_;
    pqxx::work work_{*wrapper_};
    Database db_;
};

class UnitOfWorkFactoryImpl : public UnitOfWorkFactory {
   public:
    explicit UnitOfWorkFactoryImpl(std::unique_ptr<ConnectionPool> pool);
    std::unique_ptr<UnitOfWork> CreateUnitOfWork() override;

   private:
    std::unique_ptr<ConnectionPool> pool_;
};

std::shared_ptr<UnitOfWorkFactory> CreateFactory();

}  // namespace postgres
