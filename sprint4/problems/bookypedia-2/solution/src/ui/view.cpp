#include "view.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

#include <boost/algorithm/string/trim.hpp>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    out << book.title << " by " << book.author << ", " << book.publication_year;
    return out;
}

}  // namespace detail

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}

void PrintBooks(std::ostream& out,
                const std::vector<detail::BookInfo>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value.title << ", " << value.publication_year
            << std::endl;
    }
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input,
           std::ostream& output)
    : menu_{menu}, use_cases_{use_cases}, input_{input}, output_{output} {
    menu_.AddAction("AddAuthor"s, "name"s, "Adds author"s,
                    std::bind(&View::AddAuthor, this, ph::_1));
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
                    std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s,
                    std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s,
                    std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
                    std::bind(&View::ShowAuthorBooks, this, ph::_1));
    menu_.AddAction("DeleteAuthor"s, "[author name]"s, "Deletes autor"s,
                    std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("EditAuthor"s, "[author name]"s, "Allows to edit author"s,
                    std::bind(&View::EditAuthor, this, ph::_1));
    menu_.AddAction("ShowBook"s, "[book title]"s, "Show book in detail"s,
                    std::bind(&View::ShowBook, this, ph::_1));
    menu_.AddAction("DeleteBook"s, "[book title]"s, "Delete book"s,
                    std::bind(&View::DeleteBook, this, ph::_1));
    menu_.AddAction("EditBook"s, "[book title]"s, "Allows to edit detail"s,
                    std::bind(&View::EditBook, this, ph::_1));
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);

        if (name.empty()) {
            throw std::invalid_argument("No name");
        }

        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, use_cases_.GetAuthors());
    return true;
}

bool View::ShowBooks() const {
    auto books = use_cases_.GetBooks();

    std::sort(books.begin(), books.end(),
              [](const detail::BookInfo& lhs, const detail::BookInfo& rhs) {
                  return (lhs.title < rhs.title) ||
                         (lhs.title == rhs.title && lhs.author < rhs.author) ||
                         (lhs.title == rhs.title && lhs.author == rhs.author &&
                          lhs.publication_year < rhs.publication_year);
              });

    PrintVector(output_, books);
    return true;
}

bool View::ShowAuthorBooks(std::istream& cmd_input) const {
    try {
        auto author_name = GetOneMoreArg(cmd_input);
        std::optional<detail::AuthorInfo> info;

        if (author_name.has_value()) {
            info = use_cases_.GetOptionalAuthor(*author_name);
        } else {
            info = SelectAuthor();
        }

        if (!info.has_value()) {
            throw std::invalid_argument("No author name");
        }

        auto books = use_cases_.GetAuthorBooks(info->id);

        PrintBooks(output_, books);
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}

detail::AddBookParams View::GetBookTitleAndYear(std::istream& cmd_input) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);

    if (params.title.empty()) {
        throw std::invalid_argument("Invalid book params");
    }

    return params;
}

std::optional<std::string> View::AskAuthorName() const {
    output_ << "Enter author name or empty line to select from list:"
            << std::endl;

    std::string name;

    if (!std::getline(input_, name)) {
        return std::nullopt;
    }

    boost::algorithm::trim(name);

    if (name.empty()) {
        return std::nullopt;
    }

    return name;
}

bool View::AskAddAuthor(const std::string& name) const {
    output_ << "No author found. Do you want to add " + name + " (y/n)?"
            << std::endl;

    std::string ans;

    if (!std::getline(input_, ans)) {
        return false;
    }

    boost::algorithm::trim(ans);

    if (ans.empty()) {
        return false;
    }

    return ans == "y" || ans == "Y";
}

std::vector<std::string> View::SplitStrs(const std::string& str) const {
    std::vector<std::string> result;

    char last = ' ';
    std::string cur;

    for (char c : str) {
        if (c == ' ') {
            if (last != ' ' && !cur.empty()) {
                cur.push_back(c);
            }
        } else if (c == ',') {
            if (!cur.empty() && cur.back() == ' ') {
                cur.pop_back();
            }
            if (!cur.empty()) {
                result.push_back(cur);
            }
            cur.clear();
        } else {
            cur.push_back(c);
        }
        last = c;
    }

    if (!cur.empty() && cur.back() == ' ') {
        cur.pop_back();
    }

    if (!cur.empty()) {
        result.push_back(cur);
    }

    return result;
}

std::optional<std::string> View::GetOneMoreArg(std::istream& cmd_input) const {
    std::string arg;

    if (!std::getline(cmd_input, arg)) {
        return std::nullopt;
    }

    boost::algorithm::trim(arg);

    if (arg.empty()) {
        return std::nullopt;
    }

    return arg;
}

detail::AuthorInfo View::GetEditedAuthor(detail::AuthorInfo old) const {
    output_ << "Enter new name:" << std::endl;
    auto new_name = GetOneMoreArg(input_);

    if (!new_name.has_value()) {
        return old;
    }

    boost::algorithm::trim(*new_name);

    if (!new_name->empty()) {
        old.name = *new_name;
    }

    return old;
}

detail::BookInfo View::GetEditedBook(detail::BookInfo old) const {
    output_ << "Enter new title or empty line to use the current one ("
            << old.title << "):" << std::endl;
    auto new_title = GetOneMoreArg(input_);

    if (new_title.has_value()) {
        boost::algorithm::trim(*new_title);
        if (!new_title->empty()) {
            old.title = *new_title;
        }
    }

    output_ << "Enter publication year or empty line to use the current one ("
            << old.publication_year << "):" << std::endl;
    auto new_year = GetOneMoreArg(input_);

    if (new_year.has_value()) {
        int year = std::stoi(*new_year);
        old.publication_year = year;
    }

    return old;
}

std::vector<std::string> View::GetTags() const {
    std::string tags;

    if (!std::getline(input_, tags) || tags.empty()) {
        return {};
    }

    auto res = SplitStrs(tags);

    std::sort(res.begin(), res.end());

    auto it = std::unique(res.begin(), res.end());
    res.erase(it, res.end());

    return res;
}

std::vector<std::string> View::AskTags() const {
    output_ << "Enter tags (comma separated):" << std::endl;
    return GetTags();
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        detail::AddBookParams params = GetBookTitleAndYear(cmd_input);
        auto name = AskAuthorName();

        std::optional<std::string> author_id;

        if (name.has_value()) {
            auto info = use_cases_.GetOptionalAuthor(*name);

            if (info.has_value()) {
                params.author_id = info->id;
                params.author_name = info->name;

                use_cases_.AddBook(params.title, params.author_id,
                                   params.publication_year, AskTags());
            } else {
                if (AskAddAuthor(*name)) {
                    params.author_id = use_cases_.AddAuthor(*name);
                    use_cases_.AddBook(params.title, params.author_id,
                                       params.publication_year, AskTags());
                } else {
                    throw std::invalid_argument("No such author");
                }
            }
            return true;
        }

        auto info = SelectAuthor();

        if (info.has_value()) {
            params.author_id = info->id;
            params.author_name = info->name;
            use_cases_.AddBook(params.title, params.author_id,
                               params.publication_year, AskTags());
        }

    } catch (const std::exception& ex) {
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const {
    try {
        auto author_name = GetOneMoreArg(cmd_input);
        std::optional<detail::AuthorInfo> info;

        if (author_name.has_value()) {
            info = use_cases_.GetOptionalAuthor(*author_name);
        } else {
            info = SelectAuthor();
        }

        if (!info.has_value()) {
            throw std::invalid_argument("No author name");
        }

        use_cases_.RemoveAuthor(info->id);

    } catch (...) {
        output_ << "Failed to delete author" << std::endl;
    }
    return true;
}

bool View::EditAuthor(std::istream& cmd_input) const {
    try {
        auto author_name = GetOneMoreArg(cmd_input);
        std::optional<detail::AuthorInfo> info;

        if (author_name.has_value()) {
            info = use_cases_.GetOptionalAuthor(*author_name);
        } else {
            info = SelectAuthor();
        }

        if (!info.has_value()) {
            throw std::invalid_argument("No author name");
        }

        auto edited_author = GetEditedAuthor(*info);

        use_cases_.EditAuthor(edited_author.id, edited_author.name);

    } catch (...) {
        output_ << "Failed to edit author" << std::endl;
    }
    return true;
}

bool View::ShowBook(std::istream& cmd_input) const {
    try {
        auto book = SelectBookAnyWay(cmd_input);

        if (book.has_value()) {
            auto tags = use_cases_.GetBookTags(book->id);
            PrintBook(*book, tags);
        }

    } catch (...) {
    }
    return true;
}

bool View::DeleteBook(std::istream& cmd_input) const {
    try {
        auto book = SelectBookAnyWay(cmd_input);

        if (book.has_value()) {
            use_cases_.RemoveBook(book->id);
        }
        return true;
    } catch (...) {
        output_ << "Failed to delete book" << std::endl;
        return true;
    }
}

bool View::EditBook(std::istream& cmd_input) const {
    try {
        auto book = SelectBookAnyWay(cmd_input);

        if (book.has_value()) {
            detail::BookInfo info;

            auto tags = use_cases_.GetBookTags(book->id);

            info = GetEditedBook(*book);

            output_ << "Enter tags (current tags: ";
            bool is_first = true;
            for (const auto& tag : tags) {
                if (!is_first) {
                    output_ << ", ";
                }
                is_first = false;
            }
            output_ << "):" << std::endl;

            tags = GetTags();

            use_cases_.EditBook(info.id, info.title, info.publication_year,
                                tags);

        } else {
            throw std::invalid_argument("No such book");
        }

    } catch (...) {
        output_ << "Book not found" << std::endl;
    }
    return true;
}

std::optional<detail::AddBookParams> View::GetBookParams(
    std::istream& cmd_input) const {
    detail::AddBookParams params = GetBookTitleAndYear(cmd_input);

    auto author_id = SelectAuthor();
    if (not author_id.has_value())
        return std::nullopt;
    else {
        params.author_id = author_id.value().id;
        params.author_name = author_id.value().name;
        return params;
    }
}

std::optional<detail::AuthorInfo> View::SelectAuthor() const {
    output_ << "Select author:" << std::endl;
    auto authors = use_cases_.GetAuthors();
    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx];
}

std::optional<detail::BookInfo> View::SelectBook() const {
    return SelectBook(use_cases_.GetBooks());
}

std::optional<detail::BookInfo> View::SelectBook(
    const std::vector<detail::BookInfo>& books) const {
    if (books.empty()) {
        return std::nullopt;
    }

    if (books.size() == 1) {
        return books.at(0);
    }

    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return books[book_idx];
}

std::optional<detail::BookInfo> View::SelectBookAnyWay(
    std::istream& cmd_input) const {
    auto book_name = GetOneMoreArg(cmd_input);

    if (book_name.has_value()) {
        return SelectBook(use_cases_.GetBooksByTitle(*book_name));
    }

    return SelectBook();
}

void View::PrintBook(const detail::BookInfo& book,
                     const std::vector<std::string>& tags) const {
    output_ << "Title: " << book.title << std::endl;
    output_ << "Author: " << book.author << std::endl;
    output_ << "Publication year: " << book.publication_year << std::endl;
    PrintTags(tags);
}

void View::PrintTags(const std::vector<std::string>& tags) const {
    if (tags.empty()) {
        return;
    }

    output_ << "Tags: ";

    bool is_first = true;

    for (const auto& tag : tags) {
        if (!is_first) {
            output_ << ", ";
        }
        is_first = false;
        output_ << tag;
    }
    output_ << std::endl;
}

}  // namespace ui
