#pragma once

#include <string>

#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
   public:
    explicit UseCasesImpl(domain::AuthorRepository& authors,
                          domain::BookRepository& books)
        : authors_{authors}, books_{books} {}

    void AddAuthor(const std::string& name) override;

    Authors ShowAuthors() const override;

    void AddBook(const std::string& title, const std::string& author_id,
                 int publication_year) override;

    Books ShowAuthorBooks(const std::string& author_id) const override;

    Books ShowBooks() const override;

   private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
};

}  // namespace app
