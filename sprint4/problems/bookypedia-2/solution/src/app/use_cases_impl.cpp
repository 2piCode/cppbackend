#include "use_cases_impl.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/tag.h"
#include "../ui/view.h"

namespace app {
using namespace domain;

UseCasesImpl::AuthorId UseCasesImpl::AddAuthor(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Author name can't be empty");
    }
    domain::AuthorId id = domain::AuthorId::New();
    factory_->CreateUnitOfWork()->GetAuthorRepository().Save({id, name});
    return id.ToString();
}

UseCasesImpl::Authors UseCasesImpl::GetAuthors() const {
    Authors authors;
    for (const auto& author :
         factory_->CreateUnitOfWork()->GetAuthorRepository().GetAll()) {
        authors.push_back({author.GetId().ToString(), author.GetName()});
    }
    return authors;
}

void UseCasesImpl::RemoveAuthor(const std::string& id) {
    auto ufw = factory_->CreateUnitOfWork();

    bool is_find = false;
    domain::AuthorId author_id = domain::AuthorId::FromString(id);
    for (const auto& author : ufw->GetAuthorRepository().GetAll()) {
        if (author.GetId() == author_id) {
            is_find = true;
            break;
        }
    }
    if (!is_find) {
        throw std::runtime_error("Author not found");
    }

    for (const auto& book : ufw->GetBookRepository().GetAll()) {
        if (book.GetAuthorId() == domain::AuthorId::FromString(id)) {
            for (const auto& tag :
                 ufw->GetTagRepository().GetBookTags(book.GetId())) {
                ufw->GetTagRepository().Remove(tag.GetBookId());
            }
            ufw->GetBookRepository().Remove(book.GetId());
        }
    }

    ufw->GetAuthorRepository().Remove(domain::AuthorId::FromString(id));
}

void UseCasesImpl::EditAuthor(const std::string& id, const std::string& name) {
    auto ufw = factory_->CreateUnitOfWork();
    ufw->GetAuthorRepository().Update(domain::AuthorId::FromString(id), name);
}

std::optional<ui::detail::AuthorInfo> UseCasesImpl::GetOptionalAuthor(
    std::string name) {
    auto ufw = factory_->CreateUnitOfWork();
    auto author = ufw->GetAuthorRepository().GetByName(name);
    if (!author) {
        return std::nullopt;
    }
    return ui::detail::AuthorInfo{author->GetId().ToString(),
                                  author->GetName()};
}

UseCasesImpl::AuthorId UseCasesImpl::AddBook(
    const std::string& title, const std::string& author_id,
    int publication_year, const std::vector<std::string>& tags) {
    auto book_id = domain::BookId::New();
    auto uow = factory_->CreateUnitOfWork();
    uow->GetBookRepository().Save({book_id,
                                   domain::AuthorId::FromString(author_id),
                                   title, publication_year});
    for (const auto& tag : tags) {
        uow->GetTagRepository().Save({book_id, tag});
    }
    return book_id.ToString();
}

ui::detail::BookInfo ConvertToBook(std::unique_ptr<UnitOfWork>& uow,
                                   const Book& book) {
    auto author = uow->GetAuthorRepository().Get(book.GetAuthorId());
    auto tags = uow->GetTagRepository().GetBookTags(book.GetId());
    std::vector<std::string> tags_info;
    for (const auto& tag : tags) {
        tags_info.push_back(tag.GetValue());
    }
    return {book.GetId().ToString(), book.GetTitle(), book.GetPublicationYear(),
            author.GetName()};
}

UseCasesImpl::Books ConvertToBooks(std::unique_ptr<UnitOfWork>& uow,
                                   std::vector<Book> books) {
    UseCasesImpl::Books result;
    for (const auto& book : books) {
        result.push_back(ConvertToBook(uow, book));
    }
    return result;
}

UseCasesImpl::Books UseCasesImpl::GetAuthorBooks(
    const std::string& author_id) const {
    auto uow = factory_->CreateUnitOfWork();
    return ConvertToBooks(uow, uow->GetBookRepository().ShowAuthorBooks(
                                   domain::AuthorId::FromString(author_id)));
}

UseCasesImpl::Books UseCasesImpl::GetBooksByTitle(
    const std::string& title) const {
    auto uow = factory_->CreateUnitOfWork();
    return ConvertToBooks(uow, uow->GetBookRepository().GetByTitle(title));
}

UseCasesImpl::Books UseCasesImpl::GetBooks() const {
    auto uow = factory_->CreateUnitOfWork();
    return ConvertToBooks(uow, uow->GetBookRepository().GetAll());
}

ui::detail::BookInfo UseCasesImpl::GetBook(const std::string& id) const {
    auto uow = factory_->CreateUnitOfWork();
    auto book = uow->GetBookRepository().Get(domain::BookId::FromString(id));
    return ConvertToBook(uow, book);
}

void UseCasesImpl::RemoveBook(const std::string& id) {
    auto ufw = factory_->CreateUnitOfWork();
    for (const auto& tag :
         ufw->GetTagRepository().GetBookTags(domain::BookId::FromString(id))) {
        ufw->GetTagRepository().Remove(tag.GetBookId());
    }
    ufw->GetBookRepository().Remove(domain::BookId::FromString(id));
    ufw->Commit();
}

void UseCasesImpl::EditBook(const std::string& id, const std::string& title,
                            int publication_year,
                            std::vector<std::string> tags) {
    auto ufw = factory_->CreateUnitOfWork();

    ufw->GetBookRepository().Update(id, title, publication_year);

    ufw->GetTagRepository().Remove(domain::BookId::FromString(id));
    for (const auto& tag : tags) {
        ufw->GetTagRepository().Save({domain::BookId::FromString(id), tag});
    }
}

void UseCasesImpl::AddTag(const std::string book_id, std::string value) const {
    factory_->CreateUnitOfWork()->GetTagRepository().Save(
        {domain::BookId::FromString(book_id), value});
}

std::vector<std::string> ConvertToTagsInfo(std::vector<Tag> tags) {
    std::vector<std::string> tags_infos;
    tags_infos.reserve(tags.size());
    for (const auto& tag : tags) {
        tags_infos.push_back(tag.GetValue());
    }
    return tags_infos;
}

UseCasesImpl::Tags UseCasesImpl::GetTags() const {
    auto ufw = factory_->CreateUnitOfWork();
    return ConvertToTagsInfo(ufw->GetTagRepository().GetAll());
}

UseCasesImpl::Tags UseCasesImpl::GetBookTags(std::string book_id) const {
    auto ufw = factory_->CreateUnitOfWork();
    return ConvertToTagsInfo(ufw->GetTagRepository().GetBookTags(
        domain::BookId::FromString(book_id)));
}

}  // namespace app
