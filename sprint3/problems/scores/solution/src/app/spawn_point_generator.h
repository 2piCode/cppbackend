#pragma once

#include <algorithm>
#include <random>

#include "model/map.h"
#include "model/model.h"

class SpawnPointGenerator {
   public:
    explicit SpawnPointGenerator(bool is_random_spawn_point = true)
        : is_random_spawn_point_(is_random_spawn_point) {}

    model::Coordinate Generate(const model::Map::Pointer map) {
        auto roads = map->GetRoads();

        if (!is_random_spawn_point_)
            return model::Coordinate{
                .x = static_cast<double>(roads[0]->GetStart().x),
                .y = static_cast<double>(roads[0]->GetStart().y)};

        std::uniform_int_distribution<std::uint32_t> distribution_roads(
            0, roads.size() - 1);
        auto road = roads[distribution_roads(generator1_)];

        std::uniform_int_distribution<std::int32_t> distributionX(
            std::min(road->GetStart().x, road->GetEnd().x),
            std::max(road->GetStart().x, road->GetEnd().x));
        std::uniform_int_distribution<std::int32_t> distributionY(
            std::min(road->GetStart().y, road->GetEnd().y),
            std::max(road->GetStart().y, road->GetEnd().y));

        return {static_cast<double>(distributionX(generator1_)),
                static_cast<double>(distributionY(generator1_))};
    }

   private:
    bool is_random_spawn_point_;

    std::random_device random_device_;
    std::mt19937_64 generator1_{random_device_()};
};
