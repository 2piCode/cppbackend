#include "postgres.h"

#include <pqxx/result>
#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    work_.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
}

domain::Author AuthorRepositoryImpl::Get(const domain::AuthorId& id) const {
    auto request =
        "SELECT id, name FROM authors WHERE id = " + work_.quote(id.ToString());
    const auto [_, name] = work_.query1<std::string, std::string>(request);
    return domain::Author(id, name);
}

AuthorRepositoryImpl::Authors AuthorRepositoryImpl::GetAll() const {
    auto request = R"(SELECT id, name FROM authors ORDER BY name;)"_zv;

    AuthorRepositoryImpl::Authors authors;
    for (const auto [id, name] :
         work_.query<std::string, std::string>(request)) {
        authors.emplace_back(domain::AuthorId::FromString(id), name);
    }
    return authors;
}

std::optional<domain::Author> AuthorRepositoryImpl::GetByName(
    const std::string& name) const {
    auto request =
        "SELECT id, name FROM authors WHERE name = " + work_.quote(name);
    const auto result = work_.query01<std::string, std::string>(request);
    if (!result.has_value()) {
        return std::nullopt;
    }
    const auto [id, _] = *result;
    return domain::Author(domain::AuthorId::FromString(id), name);
}

void AuthorRepositoryImpl::Remove(const domain::AuthorId& id) {
    work_.exec_params(
        R"(
DELETE FROM authors WHERE id = $1
)"_zv,
        id.ToString());
}

void AuthorRepositoryImpl::Update(const domain::AuthorId& id,
                                  const std::string& name) {
    work_.exec_params(
        R"(
UPDATE authors SET name = $2 WHERE id = $1
)"_zv,
        id.ToString(), name);
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    work_.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year)
VALUES ($1, $2, $3, $4)
)"_zv,
        book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(),
        book.GetPublicationYear());
}

BookRepositoryImpl::Books BookRepositoryImpl::GetAll() const {
    auto request =
        R"(SELECT id, author_id, title, publication_year FROM books ORDER BY title ASC)"_zv;

    Books books;
    for (const auto [id, author_id, title, year] :
         work_.query<std::string, std::string, std::string, int>(request)) {
        books.emplace_back(domain::BookId::FromString(id),
                           domain::AuthorId::FromString(author_id), title,
                           year);
    }
    return books;
}

BookRepositoryImpl::Books BookRepositoryImpl::GetByTitle(
    const std::string& title) const {
    auto request =
        "SELECT id, author_id, title, publication_year FROM books WHERE title "
        "= " +
        work_.quote(title) + " ORDER BY title;";
    Books books;
    for (const auto [id, author_id, title, year] :
         work_.query<std::string, std::string, std::string, int>(request)) {
        books.emplace_back(domain::BookId::FromString(id),
                           domain::AuthorId::FromString(author_id), title,
                           year);
    }
    return books;
}

domain::Book BookRepositoryImpl::Get(domain::BookId id) const {
    auto request =
        "SELECT id, author_id, title, publication_year FROM books WHERE id = " +
        work_.quote(id.ToString());
    const auto [_, author_id, title, year] =
        work_.query1<std::string, std::string, std::string, int>(request);
    return domain::Book(id, domain::AuthorId::FromString(author_id), title,
                        year);
}

BookRepositoryImpl::Books BookRepositoryImpl::ShowAuthorBooks(
    domain::AuthorId author_id) const {
    auto request =
        "SELECT id, author_id, title, publication_year FROM books WHERE "
        "author_id = " +
        work_.quote(author_id.ToString()) +
        " ORDER BY publication_year ASC, title ASC";

    Books books;
    for (const auto [id, author_id, title, year] :
         work_.query<std::string, std::string, std::string, int>(request)) {
        books.emplace_back(domain::BookId::FromString(id),
                           domain::AuthorId::FromString(author_id), title,
                           year);
    }
    return books;
}

void BookRepositoryImpl::Remove(domain::BookId id) {
    work_.exec_params(
        R"(
DELETE FROM books WHERE id = $1
)"_zv,
        id.ToString());
}

void BookRepositoryImpl::Update(const std::string& id, const std::string& title,
                                int publication_year) {
    work_.exec_params(
        R"(
UPDATE books SET title = $2, publication_year = $3 WHERE id = $1
)"_zv,
        id, title, publication_year);
}

void TagRepositoryImpl::Save(const domain::Tag& tag) {
    work_.exec_params(
        R"(
INSERT INTO book_tags (book_id, tag) VALUES ($1, $2)
)"_zv,
        tag.GetBookId().ToString(), tag.GetValue());
}

void TagRepositoryImpl::Remove(domain::BookId id) {
    work_.exec_params(
        R"(
DELETE FROM book_tags WHERE book_id = $1
)"_zv,
        id.ToString());
}

TagRepositoryImpl::Tags TagRepositoryImpl::GetAll() const {
    auto request = R"(SELECT book_id, tag FROM book_tags)"_zv;

    Tags tags;
    for (const auto [book_id, tag] :
         work_.query<std::string, std::string>(request)) {
        tags.emplace_back(
            domain::Tag(domain::BookId::FromString(book_id), tag));
    }
    return tags;
}

TagRepositoryImpl::Tags TagRepositoryImpl::GetBookTags(
    domain::BookId book_id) const {
    auto request = "SELECT book_id, tag FROM book_tags WHERE book_id = " +
                   work_.quote(book_id.ToString()) + " ORDER BY tag;";

    Tags tags;
    for (const auto [book_id, tag] :
         work_.query<std::string, std::string>(request)) {
        tags.emplace_back(
            domain::Tag(domain::BookId::FromString(book_id), tag));
    }
    return tags;
}

Database::Database(pqxx::work& work) : work_(work) {
    work_.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);

    work_.exec(R"(
CREATE TABLE IF NOT EXISTS books (
   id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
   author_id UUID NOT NULL,
   title varchar(100) NOT NULL,
   publication_year integer NOT NULL,
   FOREIGN KEY (author_id) REFERENCES authors (id)
);
)"_zv);

    work_.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
   book_id UUID,
   tag varchar(30)
);
)"_zv);
}

}  // namespace postgres
