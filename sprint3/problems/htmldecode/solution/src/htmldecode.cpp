#include "htmldecode.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>

std::string HtmlDecode(std::string_view str) {
    if (str.empty()) {
        return "";
    }
    static const std::unordered_map<std::string_view, char> map = {
        {"&lt", '<'},    {"&LT", '<'},   {"&gt", '>'},    {"&GT", '>'},
        {"&amp", '&'},   {"&AMP", '&'},  {"&apos", '\''}, {"&APOS", '\''},
        {"&quot", '\"'}, {"&QUOT", '\"'}};

    std::ostringstream result;

    for (size_t i = 0; i < str.size();) {
        if (str[i] != '&') {
            result << str[i++];
            continue;
        }
        bool decoded = false;
        size_t maxCheckLen = std::min<size_t>(7, str.size() - i);
        std::string candidate = std::string(str.substr(i, maxCheckLen));

        for (const auto& [key, value] : map) {
            if (candidate.size() >= key.size() &&
                candidate.compare(0, key.size(), key) == 0) {
                if (candidate[key.size()] == ';') {
                    i++;
                }
                result << value;
                i += key.size();
                decoded = true;
                break;
            }
        }
        if (!decoded) result << str[i++];
    }

    return result.str();
}
