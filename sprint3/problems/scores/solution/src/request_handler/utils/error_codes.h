#pragma once

#include <string_view>

namespace request_handler::error_codes {

inline constexpr std::string_view kInvalidMethod = "invalidMethod";
inline constexpr std::string_view kInvalidJson = "invalidJson";
inline constexpr std::string_view kBadRequest = "badRequest";
inline constexpr std::string_view kNotFound = "notFound";
inline constexpr std::string_view kInvalidToken = "invalidToken";
inline constexpr std::string_view kMapNotFound = "mapNotFound";
inline constexpr std::string_view kInvalidArgument = "invalidArgument";

}  // namespace request_handler::error_codes
