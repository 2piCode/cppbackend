#pragma once
#include <algorithm>
#include <boost/beast/http/file_body.hpp>
#include <boost/system/error_code.hpp>
#include <cctype>
#include <filesystem>
#include <iostream>

class FileHandler {
   public:
    FileHandler(std::filesystem::path static_files_root)
        : static_files_root_(
              std::filesystem::weakly_canonical(static_files_root)) {}

    template <typename Writer>
    bool GetFile(std::filesystem::path path_to_file, Writer&& writer) const {
        if (path_to_file == "/") {
            path_to_file = std::filesystem::path("/index.html");
        }
        path_to_file = static_files_root_ / path_to_file.relative_path();

        if (!IsSubPath(path_to_file, static_files_root_)) {
            return false;
        }

        boost::beast::http::file_body::value_type file;
        if (boost::system::error_code ec;
            file.open(path_to_file.c_str(), boost::beast::file_mode::read, ec),
            ec) {
            return false;
        }
        std::string extension = path_to_file.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       ::tolower);
        writer(std::move(file), extension);
        return true;
    }

   private:
    std::filesystem::path static_files_root_;

    bool IsSubPath(std::filesystem::path path,
                   std::filesystem::path base) const {
        path = std::filesystem::weakly_canonical(path);
        base = std::filesystem::weakly_canonical(base);

        for (auto b = base.begin(), p = path.begin(); b != base.end();
             ++b, ++p) {
            if (p == path.end() || *p != *b) {
                return false;
            }
        }
        return true;
    }
};
