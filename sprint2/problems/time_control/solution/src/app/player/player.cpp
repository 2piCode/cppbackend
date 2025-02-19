#include "player.h"

#include <algorithm>

namespace app {

model::Coordinate ClampPositionToRoad(const model::Road::Pointer& road,
                                      const model::Coordinate& pos) {
    auto start = road->GetStart();
    auto end = road->GetEnd();

    auto [min_x, max_x] = std::minmax(start.x, end.x);
    auto [min_y, max_y] = std::minmax(start.y, end.y);

    auto clamp = [](double val, double low, double high) {
        return std::max(low, std::min(val, high));
    };

    return {
        clamp(pos.x, min_x - model::Road::WIDTH, max_x + model::Road::WIDTH),
        clamp(pos.y, min_y - model::Road::WIDTH, max_y + model::Road::WIDTH)};
}

bool IsRoadContainsPoint(const model::Coordinate& point,
                         const model::Road::Pointer& road) {
    auto clamped = ClampPositionToRoad(road, point);
    constexpr double EPS = 1e-9;
    return (std::abs(clamped.x - point.x) < EPS) &&
           (std::abs(clamped.y - point.y) < EPS);
}

std::optional<model::Coordinate> FindClampedPositionIfOutside(
    const model::Road::Pointer& road, const model::Coordinate& new_position) {
    auto clamped = ClampPositionToRoad(road, new_position);
    constexpr double EPS = 1e-9;
    bool is_inside = (std::abs(clamped.x - new_position.x) < EPS) &&
                     (std::abs(clamped.y - new_position.y) < EPS);

    if (is_inside) {
        return std::nullopt;
    }
    return clamped;
}

void Player::Move(int delta_time) {
    auto velocity = dog_->GetVelocity();
    auto position = dog_->GetPosition();

    auto delta = static_cast<double>(delta_time) / 1000;
    auto new_position = position + velocity * delta;
    auto roads = session_->GetMap()->FindRoads(position);

    if (roads.empty()) {
        return;
    }

    for (const auto& road : roads) {
        if (IsRoadContainsPoint(new_position, road)) {
            dog_->SetPosition(new_position);
            return;
        }
    }
    MoveToBorder(new_position, roads);
}

void Player::MoveToBorder(const model::Coordinate& new_position,
                          const std::vector<model::Road::Pointer>& roads) {
    std::vector<model::Coordinate> candidates;
    candidates.reserve(roads.size());

    for (const auto& road : roads) {
        if (auto bound_pos = FindClampedPositionIfOutside(road, new_position)) {
            candidates.push_back(*bound_pos);
        }
    }

    if (candidates.empty()) {
        return;
    }

    dog_->Stop();

    auto compare_y = [](const auto& a, const auto& b) { return a.y < b.y; };
    auto compare_x = [](const auto& a, const auto& b) { return a.x < b.x; };

    model::Coordinate final_pos = candidates.front();

    switch (dog_->GetDirection()) {
        case Direction::NORTH: {
            final_pos = *std::min_element(candidates.begin(), candidates.end(),
                                          compare_y);
            break;
        }
        case Direction::SOUTH: {
            final_pos = *std::max_element(candidates.begin(), candidates.end(),
                                          compare_y);
            break;
        }
        case Direction::WEST: {
            final_pos = *std::min_element(candidates.begin(), candidates.end(),
                                          compare_x);
            break;
        }
        case Direction::EAST: {
            final_pos = *std::max_element(candidates.begin(), candidates.end(),
                                          compare_x);
            break;
        }
        default:
            break;
    }

    dog_->SetPosition(final_pos);
}

}  // namespace app
