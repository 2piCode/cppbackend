#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ui {
namespace detail {
struct AuthorInfo;
struct BookInfo;
struct AddBookParams;
}  // namespace detail
}  // namespace ui

namespace app {

class UseCases {
   public:
    using Authors = std::vector<ui::detail::AuthorInfo>;
    using Books = std::vector<ui::detail::BookInfo>;
    using Tags = std::vector<std::string>;
    using AuthorId = std::string;
    using BookId = std::string;

    virtual AuthorId AddAuthor(const std::string& name) = 0;
    virtual void RemoveAuthor(const std::string& id) = 0;
    virtual void EditAuthor(const std::string& id, const std::string& name) = 0;
    virtual Authors GetAuthors() const = 0;
    virtual std::optional<ui::detail::AuthorInfo> GetOptionalAuthor(
        std::string name) = 0;

    virtual BookId AddBook(const std::string& title,
                           const std::string& author_id, int publication_year,
                           const std::vector<std::string>& tags) = 0;
    virtual Books GetAuthorBooks(const std::string& author_id) const = 0;
    virtual Books GetBooksByTitle(const std::string& title) const = 0;
    virtual Books GetBooks() const = 0;
    virtual ui::detail::BookInfo GetBook(const std::string& id) const = 0;
    virtual void RemoveBook(const std::string& id) = 0;
    virtual void EditBook(const std::string& id, const std::string& title,
                          int publication_year,
                          std::vector<std::string> tags) = 0;

    virtual void AddTag(const std::string book_id, std::string value) const = 0;
    virtual Tags GetTags() const = 0;
    virtual Tags GetBookTags(std::string book_id) const = 0;

   protected:
    ~UseCases() = default;
};

}  // namespace app
