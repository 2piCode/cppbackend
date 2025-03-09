#pragma once

#include <vector>

#include "model/model.h"

namespace collision_detector {

struct CollectionResult {
    bool IsCollected(double collect_radius) const {
        return proj_ratio >= 0 && proj_ratio <= 1 &&
               sq_distance <= collect_radius * collect_radius;
    }

    double sq_distance;
    double proj_ratio;
};

CollectionResult TryCollectPoint(model::Coordinate a, model::Coordinate b,
                                 model::Coordinate c);

struct Gatherer {
    model::Coordinate start_pos;
    model::Coordinate end_pos;
    double width;
};

struct Item {
    model::Coordinate position;
    double width;
};

class ItemGathererProvider {
   protected:
    ~ItemGathererProvider() = default;

   public:
    virtual size_t ItemsCount() const = 0;
    virtual Item GetItem(size_t idx) const = 0;
    virtual size_t GatherersCount() const = 0;
    virtual Gatherer GetGatherer(size_t idx) const = 0;
};

struct GatheringEvent {
    size_t item_id;
    size_t gatherer_id;
    double sq_distance;
    double time;
};

std::vector<GatheringEvent> FindGatherEvents(
    const ItemGathererProvider& provider);

}  // namespace collision_detector
