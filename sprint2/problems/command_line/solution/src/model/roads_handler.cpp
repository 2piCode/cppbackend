#include "roads_handler.h"

#include <algorithm>

namespace model {

std::vector<Road::Pointer> RoadsHandler::FindRoads(Coordinate pos) const {
    std::vector<Road::Pointer> result;

    auto [low_it_y, high_it_y] = GetCandidateRangeByY(pos.y);
    for (auto it = low_it_y; it != high_it_y; ++it) {
        if (pos.x >= it->min_coord_x && pos.x <= it->max_coord_x) {
            result.push_back(it->road);
        }
    }

    auto [low_it_x, high_it_x] = GetCandidateRangeByX(pos.x);
    for (auto it = low_it_x; it != high_it_x; ++it) {
        if (pos.y >= it->min_coord_y && pos.y <= it->max_coord_y) {
            result.push_back(it->road);
        }
    }

    return result;
}

void RoadsHandler::InitializeSegments() {
    for (const auto& road : roads_) {
        auto start = road->GetStart();
        auto end = road->GetEnd();
        auto [min_x, max_x] = std::minmax(start.x, end.x);
        auto [min_y, max_y] = std::minmax(start.y, end.y);

        if (road->IsHorizontal()) {
            horizontal_segments_.emplace_back(
                min_x - Road::WIDTH, max_x + Road::WIDTH, min_y - Road::WIDTH,
                max_y + Road::WIDTH, road);
            continue;
        }
        vertical_segments_.emplace_back(
            min_x - Road::WIDTH, max_x + Road::WIDTH, min_y - Road::WIDTH,
            max_y + Road::WIDTH, road);
    }

    std::sort(horizontal_segments_.begin(), horizontal_segments_.end(),
              [](const Segment& a, const Segment& b) {
                  return a.min_coord_y < b.min_coord_y;
              });

    std::sort(vertical_segments_.begin(), vertical_segments_.end(),
              [](const Segment& a, const Segment& b) {
                  return a.min_coord_x < b.min_coord_x;
              });
}

std::pair<RoadsHandler::Segments::const_iterator,
          RoadsHandler::Segments::const_iterator>
RoadsHandler::GetCandidateRangeByY(double pos_y) const {
    auto lower = std::lower_bound(
        horizontal_segments_.begin(), horizontal_segments_.end(), pos_y,
        [](auto const& seg, double y) { return seg.max_coord_y < y; });

    auto upper = std::upper_bound(
        lower, horizontal_segments_.end(), pos_y,
        [](double y, auto const& seg) { return y < seg.min_coord_y; });

    return {lower, upper};
}

std::pair<RoadsHandler::Segments::const_iterator,
          RoadsHandler::Segments::const_iterator>
RoadsHandler::GetCandidateRangeByX(double pos_x) const {
    auto lower = std::lower_bound(
        vertical_segments_.begin(), vertical_segments_.end(), pos_x,
        [](auto const& seg, double x) { return seg.max_coord_x < x; });

    auto upper = std::upper_bound(
        lower, vertical_segments_.end(), pos_x,
        [](double x, auto const& seg) { return x < seg.min_coord_x; });

    return {lower, upper};
}

}  // namespace model
