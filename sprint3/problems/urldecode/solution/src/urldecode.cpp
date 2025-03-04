#include "urldecode.h"

#include <charconv>
#include <stdexcept>

std::string UrlDecode(std::string_view str) {
    if (str.empty()) {
        return {};
    }

    std::string decoded_str;
    decoded_str.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 >= str.size() || !std::isxdigit(str[i + 1]) ||
                !std::isxdigit(str[i + 2])) {
                throw std::invalid_argument("Invalid URL encoding");
            }
            std::string hex_str = std::string(str.substr(i + 1, 2));
            int value = std::stoi(hex_str, nullptr, 16);
            decoded_str.push_back(static_cast<char>(value));
            i += 2;
        } else if (str[i] == '+') {
            decoded_str.push_back(' ');
        } else {
            decoded_str.push_back(str[i]);
        }
    }
    return decoded_str;
}
