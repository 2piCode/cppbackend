#pragma once

#include "../app/unit_of_work.h"
#include "postgres.h"

namespace postgres {

class UnitOfWorkImpl : public app::UnitOfWork {
   public:
    explicit UnitOfWorkImpl(pqxx::connection& connection)
        : connection_(connection), work_(connection_), db_{work_} {}

    ~UnitOfWorkImpl() override { Commit(); }

    void Commit() override { work_.commit(); }

    domain::BookRepository& GetBookRepository() override {
        return db_.GetBooks();
    }

    domain::AuthorRepository& GetAuthorRepository() override {
        return db_.GetAuthors();
    }

    domain::TagRepository& GetTagRepository() override { return db_.GetTags(); }

    void SaveBook(const domain::Book& book) { db_.GetBooks().Save(book); }

    void SaveAuthor(const domain::Author& author) {
        db_.GetAuthors().Save(author);
    }

   private:
    pqxx::connection& connection_;
    pqxx::work work_;
    Database db_;
};

}  // namespace postgres
