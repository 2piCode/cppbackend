#pragma once

#include <memory>
#include <unordered_map>

#include "model/map.h"

namespace serialization {
class LootNumberMapHandlerRepr;
}

class LootNumberMapHandler {
   public:
    friend class serialization::LootNumberMapHandlerRepr;

    using Pointer = std::shared_ptr<LootNumberMapHandler>;
    using LootNumberByMap =
        std::unordered_map<model::Map::Id, int,
                           util::TaggedHasher<model::Map::Id>>;

    LootNumberMapHandler(LootNumberByMap max_loot_number_by_map,
                         int base_max_loot_number)
        : max_loot_number_by_map_(max_loot_number_by_map),
          base_max_loot_number_(base_max_loot_number) {}

    int GetMaxLootNumber(const model::Map::Id& map_id) {
        if (auto it = max_loot_number_by_map_.find(map_id);
            it != max_loot_number_by_map_.end()) {
            return it->second;
        }

        return base_max_loot_number_;
    }

   private:
    LootNumberByMap max_loot_number_by_map_;
    int base_max_loot_number_;
};
