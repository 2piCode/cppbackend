#include "file_handler.h"

namespace file_handler {

FileHandler::FileHandler(std::filesystem::path static_files_root)
    : static_files_root_(std::filesystem::weakly_canonical(static_files_root)) {
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

}  // namespace file_handler
