#pragma once

#include <string>
#include <vector>

#include "../domain/author_fwd.h"
#include "../util/tagged_uuid.h"

namespace domain {

namespace detail {
struct AuthorTag {};
}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
   public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id)), name_(std::move(name)) {}

    const AuthorId& GetId() const noexcept { return id_; }

    const std::string& GetName() const noexcept { return name_; }

   private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
   public:
    using Authors = std::vector<Author>;

    virtual void Save(const Author& author) = 0;

    virtual Authors GetAll() const = 0;

   protected:
    ~AuthorRepository() = default;
};

}  // namespace domain
