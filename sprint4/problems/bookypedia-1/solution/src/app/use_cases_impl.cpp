#include "use_cases_impl.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Author name can't be empty");
    }
    authors_.Save({AuthorId::New(), name});
}

UseCasesImpl::Authors UseCasesImpl::ShowAuthors() const {
    Authors authors;
    for (const auto& author : authors_.GetAll()) {
        authors.push_back({author.GetId().ToString(), author.GetName()});
    }
    return authors;
}

void UseCasesImpl::AddBook(const std::string& title,
                           const std::string& author_id, int publication_year) {
    books_.Save({BookId::New(), AuthorId::FromString(author_id), title,
                 publication_year});
}

UseCasesImpl::Books ConvertToBooks(const std::vector<Book>& books) {
    UseCasesImpl::Books result;
    for (const auto& book : books) {
        result.push_back({book.GetId().ToString(),
                          book.GetAuthorId().ToString(), book.GetTitle(),
                          book.GetPublicationYear()});
    }
    return result;
}

UseCasesImpl::Books UseCasesImpl::ShowAuthorBooks(
    const std::string& author_id) const {
    return ConvertToBooks(
        books_.ShowAuthorBooks(AuthorId::FromString(author_id)));
}

UseCasesImpl::Books UseCasesImpl::ShowBooks() const {
    return ConvertToBooks(books_.GetAll());
}

}  // namespace app
