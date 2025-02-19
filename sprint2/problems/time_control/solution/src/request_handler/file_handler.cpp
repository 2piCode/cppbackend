#include "file_handler.h"

#include <algorithm>
#include <utility>

namespace request_handler::file_handler {

FileHandler::FileHandler(std::filesystem::path static_files_root)
    : static_files_root_(std::filesystem::weakly_canonical(static_files_root)) {
}

std::optional<FileResponse> FileHandler::operator()(
    std::filesystem::path path_to_file) const {
    if (path_to_file == "/") {
        path_to_file = std::filesystem::path("/index.html");
    }

    path_to_file = std::filesystem::path(DecodePath(path_to_file.string()));
    path_to_file = static_files_root_ / path_to_file.relative_path();

    if (!IsSubPath(path_to_file, static_files_root_)) {
        return std::nullopt;
    }

    boost::beast::http::file_body::value_type file;
    if (boost::system::error_code ec;
        file.open(std::filesystem::weakly_canonical(path_to_file).c_str(),
                  boost::beast::file_mode::read, ec),
        ec) {
        return std::nullopt;
    }

    std::string extension = path_to_file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   ::tolower);
    return FileResponse{.file = std::move(file), .extension = extension};
}

bool FileHandler::IsSubPath(std::filesystem::path path,
                            std::filesystem::path base) const {
    path = std::filesystem::weakly_canonical(path);
    base = std::filesystem::weakly_canonical(base);

    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            return false;
        }
    }
    return true;
}

std::string FileHandler::DecodePath(std::string&& path_str) const {
    std::string decoded_str;
    decoded_str.reserve(path_str.size());

    for (size_t i = 0; i < path_str.size(); ++i) {
        if (path_str[i] == '%' && i + 2 < path_str.size() &&
            std::isxdigit(path_str[i + 1]) && std::isxdigit(path_str[i + 2])) {
            std::string hex_str = path_str.substr(i + 1, 2);
            int value = std::stoi(hex_str, nullptr, 16);
            decoded_str.push_back(static_cast<char>(value));
            i += 2;
        } else if (path_str[i] == '+') {
            decoded_str.push_back(' ');
        } else {
            decoded_str.push_back(path_str[i]);
        }
    }
    return decoded_str;
}

}  // namespace request_handler::file_handler
