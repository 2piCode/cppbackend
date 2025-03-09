#pragma once

#include "model/model.h"
#include "model/tagged.h"

namespace game {

struct Item {
    using Id = util::Tagged<std::uint32_t, Item>;

    Id id;
    int type;
    model::Coordinate position;
    int value;
};

static constexpr double ItemWidth = 0.0;

}  // namespace game
