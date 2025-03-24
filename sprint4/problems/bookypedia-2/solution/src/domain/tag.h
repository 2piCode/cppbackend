#pragma once

#include <string>
#include <utility>
#include <vector>

#include "book.h"

namespace domain {

class Tag {
   public:
    Tag(BookId book_id, std::string value)
        : book_id_(std::move(book_id)), value_(std::move(value)) {}

    const BookId& GetBookId() const noexcept { return book_id_; }
    const std::string& GetValue() const noexcept { return value_; }

   private:
    BookId book_id_;
    std::string value_;
};

class TagRepository {
   public:
    using Tags = std::vector<Tag>;

    virtual void Save(const Tag& tag) = 0;
    virtual void Remove(BookId book_id) = 0;
    virtual Tags GetAll() const = 0;
    virtual Tags GetBookTags(BookId book_id) const = 0;

   protected:
    ~TagRepository() = default;
};

}  // namespace domain
