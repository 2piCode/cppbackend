#include <iostream>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>

#include <boost/json.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/src.hpp>

#define BOOST_USE

using namespace std::literals;
using pqxx::operator"" _zv;

constexpr static auto TAG_INS_BOOK = "ins_book"_zv;

bool ProcessRequest(boost::json::value js_value, pqxx::connection& conn) {
    boost::json::object js_object = js_value.as_object();
    boost::json::string action = js_object["action"].as_string();
    if (action == "add_book") {
        boost::json::object payload_object = js_object["payload"].as_object();
        std::string title = payload_object["title"].as_string().c_str();
        std::string author = payload_object["author"].as_string().c_str();
        int year = payload_object["year"].as_int64();

        std::optional<std::string> isbn = std::nullopt;
        if (auto isbn_it = payload_object.find("ISBN");
            isbn_it != payload_object.end()) {
            if (isbn_it->value().is_string()) {
                isbn = isbn_it->value().as_string();
            }
        }
        try {
            pqxx::work w(conn);

            w.exec_prepared(TAG_INS_BOOK, title, author, year, isbn);

            w.commit();

            boost::json::object result = {{"result", true}};
            std::cout << result << std::endl;

        } catch (const pqxx::sql_error& error) {
            boost::json::object result = {{"result", false}};
            std::cout << result << std::endl;
        }

    } else if (action == "all_books") {
        pqxx::read_transaction r(conn);
        boost::json::array array;
        for (auto [id, title, author, year, isbn] :
             r.query<int, std::string, std::string, int,
                     std::optional<std::string>>(
                 "SELECT id, title, author, year, ISBN FROM books;"_zv)) {
            boost::json::object result;
            result["id"] = id;
            result["title"] = title;
            result["author"] = author;
            result["year"] = year;
            if (isbn) {
                result["ISBN"] = *isbn;
            } else {
                result["ISBN"] = nullptr;
            }
            array.push_back(result);
        }
        std::cout << array << std::endl;

    } else if (action == "exit") {
        return true;
    } else {
        throw std::invalid_argument("invalid action: " +
                                    std::string(action.c_str()));
    }
    return false;
}

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: book_manager <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }

        pqxx::connection conn{argv[1]};
        pqxx::work w(conn);

        w.exec(
            "CREATE TABLE IF NOT EXISTS books (id SERIAL PRIMARY KEY, title varchar(100) NOT NULL, author varchar(100) NOT NULL, year integer NOT NULL, ISBN char(13) UNIQUE);"_zv);
        w.exec("DELETE FROM books;"_zv);

        conn.prepare(
            TAG_INS_BOOK,
            "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, $4)"_zv);

        w.commit();

        std::string request;
        while (std::getline(std::cin, request)) {
            if (ProcessRequest(boost::json::parse(request), conn)) {
                break;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
