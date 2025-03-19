#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/json/array.hpp>

#include "model/map.h"
#include "model/tagged.h"

namespace serialization {
class LootHandlerRepr;
}

class LootHandler {
   public:
    friend class serialization::LootHandlerRepr;

    using Pointer = std::shared_ptr<LootHandler>;
    using LootTypeByMap =
        std::unordered_map<model::Map::Id, boost::json::array,
                           util::TaggedHasher<model::Map::Id>>;
    using LootTypeScoreByMap =
        std::unordered_map<model::Map::Id, std::vector<int>,
                           util::TaggedHasher<model::Map::Id>>;

    explicit LootHandler(LootTypeByMap map_loot_types,
                         LootTypeScoreByMap loot_type_score)
        : map_loot_types_(std::move(map_loot_types)),
          map_score_types_(std::move(loot_type_score)) {}

    std::optional<boost::json::array> FindLootType(
        const model::Map::Id& map_id) {
        if (auto it = map_loot_types_.find(map_id);
            it != map_loot_types_.end()) {
            return it->second;
        }

        return std::nullopt;
    }

    int FindValueByLootType(const model::Map::Id& map_id, int type) {
        if (auto it = map_score_types_.find(map_id);
            it != map_score_types_.end()) {
            if (type <= it->second.size()) {
                return it->second[type];
            }
        }
        throw std::runtime_error("Invalid type for item");
    }

   private:
    LootTypeByMap map_loot_types_;
    LootTypeScoreByMap map_score_types_;
};
