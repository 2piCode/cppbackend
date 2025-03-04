#pragma once

#include <optional>
#include <vector>

#include "model/model.h"

namespace model {

bool IsRoadContainsPoint(const model::Coordinate& point,
                         const model::Road::Pointer& road);

std::optional<model::Coordinate> FindClampedPositionIfOutside(
    const model::Road::Pointer& road, const model::Coordinate& new_position);

class RoadsHandler {
   public:
    using Roads = std::vector<std::shared_ptr<Road>>;

    explicit RoadsHandler(Roads roads) noexcept : roads_{std::move(roads)} {
        InitializeSegments();
    }

    const Roads& GetRoads() const noexcept { return roads_; }

    std::vector<Road::Pointer> FindRoads(Coordinate pos) const;

   private:
    struct Segment {
        double min_coord_x, max_coord_x;
        double min_coord_y, max_coord_y;
        Road::Pointer road;
    };
    using Segments = std::vector<Segment>;
    Roads roads_;
    Segments vertical_segments_, horizontal_segments_;

    void InitializeSegments();

    std::pair<Segments::const_iterator, Segments::const_iterator>
    GetCandidateRangeByY(double pos_y) const;

    std::pair<Segments::const_iterator, Segments::const_iterator>
    GetCandidateRangeByX(double pos_x) const;
};

}  // namespace model
