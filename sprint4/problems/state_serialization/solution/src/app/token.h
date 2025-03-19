#pragma once

#include <memory>
#include <string>

#include "model/tagged.h"

namespace app {

namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

namespace token {
static constexpr size_t SIZE = 32;
using Pointer = Token*;
}  // namespace token

}  // namespace app
