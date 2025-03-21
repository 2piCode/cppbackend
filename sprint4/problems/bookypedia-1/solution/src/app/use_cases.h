#pragma once

#include <string>
#include <vector>

namespace app {

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string id;
    std::string author_id;
    std::string title;
    int publication_year;
};

class UseCases {
   public:
    using Authors = std::vector<AuthorInfo>;
    using Books = std::vector<BookInfo>;

    virtual void AddAuthor(const std::string& name) = 0;

    virtual Authors ShowAuthors() const = 0;

    virtual void AddBook(const std::string& title, const std::string& author_id,
                         int publication_year) = 0;

    virtual Books ShowAuthorBooks(const std::string& author_id) const = 0;

    virtual Books ShowBooks() const = 0;

   protected:
    ~UseCases() = default;
};

}  // namespace app
