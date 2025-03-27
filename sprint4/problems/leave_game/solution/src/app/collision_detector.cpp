#include "collision_detector.h"

#include <algorithm>
#include <cassert>

#include "model/model.h"

namespace collision_detector {

CollectionResult TryCollectPoint(model::Coordinate a, model::Coordinate b,
                                 model::Coordinate c) {
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent> FindGatherEvents(
    const ItemGathererProvider& provider) {
    std::vector<GatheringEvent> result;

    for (size_t g = 0; g < provider.GatherersCount(); g++) {
        Gatherer gatherer = provider.GetGatherer(g);
        if (gatherer.start_pos == gatherer.end_pos) {
            continue;
        }
        for (size_t i = 0; i < provider.ItemsCount(); i++) {
            Item item = provider.GetItem(i);
            auto collection_result = TryCollectPoint(
                gatherer.start_pos, gatherer.end_pos, item.position);

            if (collection_result.IsCollected(gatherer.width + item.width)) {
                result.push_back(
                    GatheringEvent{.item_id = i,
                                   .gatherer_id = g,
                                   .sq_distance = collection_result.sq_distance,
                                   .time = collection_result.proj_ratio});
            }
        }
    }

    std::function<bool(GatheringEvent lhs, GatheringEvent rhs)>
        compare_by_time = [](GatheringEvent lhs, GatheringEvent rhs) {
            return lhs.time < rhs.time;
        };

    std::sort(result.begin(), result.end(), compare_by_time);

    return result;
}

}  // namespace collision_detector
