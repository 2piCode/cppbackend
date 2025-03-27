#pragma once

#include "model/model.h"
#include "model/tagged.h"

namespace model {

struct Item {
    using Id = util::Tagged<std::uint32_t, Item>;

    Id id = Id{0u};
    int type;
    model::Coordinate position;
    int value;

    bool operator==(const Item& item) const = default;
};

static constexpr double ItemWidth = 0.0;

}  // namespace model
