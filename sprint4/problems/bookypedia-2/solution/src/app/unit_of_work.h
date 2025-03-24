#pragma once

#include <memory>

#include "../domain/domain_fwd.h"

namespace app {

class UnitOfWork {
   public:
    virtual ~UnitOfWork() = default;

    virtual void Commit() = 0;
    virtual domain::BookRepository &GetBookRepository() = 0;
    virtual domain::AuthorRepository &GetAuthorRepository() = 0;
    virtual domain::TagRepository &GetTagRepository() = 0;
};

class UnitOfWorkFactory {
   public:
    virtual ~UnitOfWorkFactory() = default;
    virtual std::unique_ptr<UnitOfWork> CreateUnitOfWork() const = 0;
};

}  // namespace app
