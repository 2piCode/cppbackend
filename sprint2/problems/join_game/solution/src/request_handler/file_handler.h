#pragma once

#include <algorithm>
#include <boost/beast/http/file_body.hpp>
#include <boost/system/error_code.hpp>
#include <cctype>
#include <filesystem>

namespace file_handler {

class FileHandler {
   public:
    FileHandler(std::filesystem::path static_files_root);

    template <typename Writer>
    bool operator()(std::filesystem::path path_to_file, Writer&& writer) const {
        if (path_to_file == "/") {
            path_to_file = std::filesystem::path("/index.html");
        }
        path_to_file = std::filesystem::path(DecodePath(path_to_file.string()));
        path_to_file = static_files_root_ / path_to_file.relative_path();

        if (!IsSubPath(path_to_file, static_files_root_)) {
            return false;
        }

        boost::beast::http::file_body::value_type file;
        if (boost::system::error_code ec;
            file.open(std::filesystem::weakly_canonical(path_to_file).c_str(),
                      boost::beast::file_mode::read, ec),
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
                   std::filesystem::path base) const;

    std::string DecodePath(std::string&& path_str) const;
};

}  // namespace file_handler
