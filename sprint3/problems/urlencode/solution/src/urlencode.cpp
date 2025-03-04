#include "urlencode.h"

#include <iomanip>
#include <sstream>

std::string UrlEncode(std::string_view str) {
    if (str.empty()) {
        return {};
    }

    std::ostringstream os;

    for (const char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            os << c;
        } else if (c == ' ') {
            os << '+';
        } else {
            os << '%' << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(c);
        }
    }

    return os.str();
}
