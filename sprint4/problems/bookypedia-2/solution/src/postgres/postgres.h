#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/tag.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
   public:
    explicit AuthorRepositoryImpl(pqxx::work& work) : work_{work} {}

    void Save(const domain::Author& author) override;
    domain::Author Get(const domain::AuthorId& id) const override;
    std::optional<domain::Author> GetByName(
        const std::string& name) const override;
    Authors GetAll() const override;
    void Remove(const domain::AuthorId& id) override;
    void Update(const domain::AuthorId& id, const std::string& name) override;

   private:
    pqxx::work& work_;
};

class BookRepositoryImpl : public domain::BookRepository {
   public:
    explicit BookRepositoryImpl(pqxx::work& work) : work_{work} {}

    void Save(const domain::Book& book) override;
    Books GetAll() const override;
    Books GetByTitle(const std::string& title) const override;
    domain::Book Get(domain::BookId id) const override;
    Books ShowAuthorBooks(domain::AuthorId author_id) const override;
    void Remove(domain::BookId id) override;
    void Update(const std::string& id, const std::string& title,
                int publication_year) override;

   private:
    pqxx::work& work_;
};

class TagRepositoryImpl : public domain::TagRepository {
   public:
    explicit TagRepositoryImpl(pqxx::work& work) : work_{work} {}

    void Save(const domain::Tag& tag) override;
    void Remove(domain::BookId id) override;
    Tags GetBookTags(domain::BookId id) const override;
    Tags GetAll() const override;

   private:
    pqxx::work& work_;
};

class Database {
   public:
    explicit Database(pqxx::work& work);

    AuthorRepositoryImpl& GetAuthors() & { return authors_; }
    BookRepositoryImpl& GetBooks() & { return books_; }
    TagRepositoryImpl& GetTags() & { return tags_; }

   private:
    pqxx::work& work_;
    AuthorRepositoryImpl authors_{work_};
    BookRepositoryImpl books_{work_};
    TagRepositoryImpl tags_{work_};
};

}  // namespace postgres
