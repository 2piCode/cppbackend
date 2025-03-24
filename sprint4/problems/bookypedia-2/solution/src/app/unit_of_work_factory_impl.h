#pragma once

#include <memory>
#include <pqxx/connection>
#include <pqxx/transaction>
#include <utility>

#include "../postgres/unit_of_work_impl.h"

namespace app {

class UnitOfWorkFactoryImpl : public UnitOfWorkFactory {
   public:
    explicit UnitOfWorkFactoryImpl(pqxx::connection connection)
        : connection_(std::move(connection)) {}

    std::unique_ptr<UnitOfWork> CreateUnitOfWork() const override {
        return std::make_unique<postgres::UnitOfWorkImpl>(
            const_cast<pqxx::connection&>(connection_));
    }

   private:
    pqxx::connection connection_;
};

}  // namespace app
