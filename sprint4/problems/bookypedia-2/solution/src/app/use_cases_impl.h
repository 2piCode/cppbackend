#pragma once

#include <memory>
#include <string>
#include <utility>

#include "unit_of_work.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
   public:
    explicit UseCasesImpl(std::unique_ptr<UnitOfWorkFactory> factory)
        : factory_(std::move(factory)) {}

    AuthorId AddAuthor(const std::string& name) override;
    Authors GetAuthors() const override;
    void RemoveAuthor(const std::string& id) override;
    void EditAuthor(const std::string& id, const std::string& name) override;
    std::optional<ui::detail::AuthorInfo> GetOptionalAuthor(
        std::string name) override;

    BookId AddBook(const std::string& title, const std::string& author_id,
                   int publication_year,
                   const std::vector<std::string>& tags) override;
    Books GetAuthorBooks(const std::string& author_id) const override;
    Books GetBooksByTitle(const std::string& title) const override;
    Books GetBooks() const override;
    ui::detail::BookInfo GetBook(const std::string& id) const override;
    void RemoveBook(const std::string& id) override;
    void EditBook(const std::string& id, const std::string& title,
                  int publication_year, std::vector<std::string> tags) override;

    void AddTag(const std::string book_id, std::string value) const override;
    Tags GetTags() const override;
    Tags GetBookTags(std::string book_id) const override;

   private:
    std::unique_ptr<UnitOfWorkFactory> factory_;
};

}  // namespace app
