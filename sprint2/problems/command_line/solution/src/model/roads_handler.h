#pragma once

#include <vector>

#include "model/model.h"

namespace model {

class RoadsHandler {
   public:
    using Roads = std::vector<std::shared_ptr<Road>>;

    RoadsHandler(Roads roads) noexcept : roads_{std::move(roads)} {
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
