#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include <boost/json/array.hpp>

#include "model/map.h"
#include "model/tagged.h"

class LootHandler {
   public:
    using Pointer = std::shared_ptr<LootHandler>;
    using LootTypeByMap =
        std::unordered_map<model::Map::Id, boost::json::array,
                           util::TaggedHasher<model::Map::Id>>;

    explicit LootHandler(LootTypeByMap&& map_loot_types)
        : map_loot_types_(std::move(map_loot_types)) {}

    std::optional<boost::json::array> FindLootType(
        const model::Map::Id& map_id) {
        if (auto it = map_loot_types_.find(map_id);
            it != map_loot_types_.end()) {
            return it->second;
        }

        return std::nullopt;
    }

   private:
    LootTypeByMap map_loot_types_;
};
