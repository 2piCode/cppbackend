#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>

#include "../src/app/unit_of_work.h"
#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"
#include "../src/domain/book.h"

/*namespace {*/
/**/
/*struct MockAuthorRepository : domain::AuthorRepository {*/
/*    std::vector<domain::Author> saved_authors;*/
/**/
/*    void Save(const domain::Author& author) override {*/
/*        saved_authors.emplace_back(author);*/
/*    }*/
/**/
/*    domain::AuthorRepository::Authors GetAll() const override {*/
/*        return saved_authors;*/
/*    }*/
/*};*/
/**/
/*struct MockBookRepository : domain::BookRepository {*/
/*    std::vector<domain::Book> saved_books;*/
/**/
/*    void Save(const domain::Book& book) override {*/
/*        saved_books.emplace_back(book);*/
/*    }*/
/**/
/*    domain::BookRepository::Books GetAll() const override {*/
/*        return saved_books;*/
/*    }*/
/**/
/*    domain::BookRepository::Books ShowAuthorBooks(*/
/*        domain::AuthorId author_id) const override {*/
/*        return saved_books;*/
/*    }*/
/*};*/
/**/
/*static MockAuthorRepository authors;*/
/*static MockBookRepository books;*/
/**/
/*struct MockUnitOfWork : app::UnitOfWork {*/
/*    void Commit() override {}*/
/**/
/*    domain::AuthorRepository& GetAuthorRepository() override { return authors;
 * }*/
/*    domain::BookRepository& GetBookRepository() override { return books; }*/
/*};*/
/**/
/*struct MockUnitOfWorkFactory : app::UnitOfWorkFactory {*/
/*    std::unique_ptr<app::UnitOfWork> CreateUnitOfWork() const override {*/
/*        return std::make_unique<MockUnitOfWork>();*/
/*    }*/
/*};*/
/**/
/*}  // namespace*/
/**/
/*SCENARIO("Book Adding") {*/
/*    authors.saved_authors.clear();*/
/*    GIVEN("Use cases") {*/
/*        app::UseCasesImpl
 * use_cases{std::make_unique<MockUnitOfWorkFactory>()};*/
/**/
/*        WHEN("Adding an author") {*/
/*            const auto author_name = "Joanne Rowling";*/
/*            use_cases.AddAuthor(author_name);*/
/**/
/*            THEN("author with the specified name is saved to repository") {*/
/*                REQUIRE(authors.saved_authors.size() == 1);*/
/*                CHECK(authors.saved_authors.at(0).GetName() == author_name);*/
/*                CHECK(authors.saved_authors.at(0).GetId() !=*/
/*                      domain::AuthorId{});*/
/**/
/*                AND_WHEN("show an authors") {*/
/*                    AND_THEN("output contains author") {*/
/*                        auto result = use_cases.GetAuthors();*/
/*                        CHECK(result.size() == 1);*/
/*                        CHECK(result.at(0).id ==*/
/*                              authors.saved_authors.at(0).GetId().ToString());*/
/*                        CHECK(result.at(0).name ==*/
/*                              authors.saved_authors.at(0).GetName());*/
/*                    }*/
/*                }*/
/**/
/*                AND_WHEN("adding a book with selected author") {*/
/*                    const auto book_title = "Harry Potter";*/
/*                    const int year = 1998;*/
/**/
/*                    use_cases.AddBook(*/
/*                        book_title,*/
/*                        authors.saved_authors.at(0).GetId().ToString(),
 * year);*/
/*                    THEN("book is saved to repository") {*/
/*                        REQUIRE(books.saved_books.size() == 1);*/
/*                        CHECK(books.saved_books.at(0).GetTitle() ==
 * book_title);*/
/*                        CHECK(books.saved_books.at(0).GetAuthorId() ==*/
/*                              authors.saved_authors.at(0).GetId());*/
/*                        CHECK(books.saved_books.at(0).GetPublicationYear()
 * ==*/
/*                              year);*/
/*                    }*/
/*                }*/
/*            }*/
/*        }*/
/**/
/*        WHEN("try to add author with the empty name") {*/
/*            CHECK_THROWS_AS(use_cases.AddAuthor(""), std::invalid_argument);*/
/*        }*/
/*    }*/
/*}*/
