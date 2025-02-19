#pragma once

#include <string>
//
#include <boost/json.hpp>

inline std::optional<boost::json::object> Parse(const std::string& body) {
    boost::system::error_code ec;
    const boost::json::value json_body_value = boost::json::parse(body, ec);

    if (ec) {
        return std::nullopt;
    }

    return json_body_value.as_object();
}
