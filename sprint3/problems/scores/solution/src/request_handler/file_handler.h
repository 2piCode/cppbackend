#pragma once

#include <cctype>
#include <filesystem>
#include <optional>
#include <string>

#include <boost/beast/http/file_body.hpp>
#include <boost/system/error_code.hpp>

namespace request_handler {

namespace file_handler {

struct FileResponse {
    boost::beast::http::file_body::value_type file;
    std::string extension;
};

bool IsSubPath(std::filesystem::path path, std::filesystem::path base);

class FileHandler {
   public:
    explicit FileHandler(std::filesystem::path static_files_root);

    std::optional<FileResponse> operator()(
        std::filesystem::path path_to_file) const;

   private:
    std::filesystem::path static_files_root_;

    std::string DecodePath(std::string&& path_str) const;
};

}  // namespace file_handler

}  // namespace request_handler
