#include "postgres.h"

#include <pqxx/result>
#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;
void AuthorRepositoryImpl::Save(const domain::Author& author) {
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

AuthorRepositoryImpl::Authors AuthorRepositoryImpl::GetAll() const {
    pqxx::read_transaction read{connection_};
    auto request = R"(SELECT id, name FROM authors ORDER BY name ASC)"_zv;
    pqxx::result result = read.exec(request);

    AuthorRepositoryImpl::Authors authors;
    for (const auto [id, title] :
         read.query<std::string, std::string>(request)) {
        authors.emplace_back(domain::AuthorId::FromString(id), title);
    }
    return authors;
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year)
VALUES ($1, $2, $3, $4)
ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
)"_zv,
        book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(),
        book.GetPublicationYear());
    work.commit();
}

BookRepositoryImpl::Books BookRepositoryImpl::GetAll() const {
    pqxx::read_transaction read{connection_};
    auto request =
        R"(SELECT id, author_id, title, publication_year FROM books ORDER BY title ASC)"_zv;

    Books books;
    for (const auto [id, author_id, title, year] :
         read.query<std::string, std::string, std::string, int>(request)) {
        books.emplace_back(domain::BookId::FromString(id),
                           domain::AuthorId::FromString(author_id), title,
                           year);
    }
    return books;
}

BookRepositoryImpl::Books BookRepositoryImpl::ShowAuthorBooks(
    domain::AuthorId author_id) const {
    pqxx::read_transaction read{connection_};
    auto request =
        "SELECT id, author_id, title, publication_year FROM books WHERE "
        "author_id = " +
        read.quote(author_id.ToString()) +
        "ORDER BY publication_year ASC, title ASC";

    Books books;
    for (const auto [id, author_id, title, year] :
         read.query<std::string, std::string, std::string, int>(request)) {
        books.emplace_back(domain::BookId::FromString(id),
                           domain::AuthorId::FromString(author_id), title,
                           year);
    }
    return books;
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);

    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
   id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
   author_id UUID NOT NULL,
   title varchar(100) NOT NULL,
   publication_year integer NOT NULL,
   FOREIGN KEY (author_id) REFERENCES authors (id)
);
)"_zv);

    // коммитим изменения
    work.commit();
}

}  // namespace postgres
