#include "player.h"

#include <algorithm>

#include "model/roads_handler.h"

namespace app {

MovementInfo Player::Move(std::chrono::milliseconds delta_time) {
    using namespace std::chrono_literals;
    dog_->SetTimeInGame(dog_->GetTimeInGame() + delta_time);

    if (dog_->GetVelocity().x == 0 && dog_->GetVelocity().y == 0) {
        dog_->SetLastMoveTime(dog_->GetLastMoveTime() + delta_time);
    } else {
        dog_->SetLastMoveTime(0ms);
    }

    auto velocity = dog_->GetVelocity();
    auto position = dog_->GetPosition();

    auto delta = delta_time.count() / 1000;
    auto new_position = position + velocity * delta;
    auto roads = session_->GetMap()->FindRoads(position);

    if (roads.empty()) {
        return MovementInfo{.start_position = dog_->GetPosition(),
                            .end_position = dog_->GetPosition()};
    }

    if (std::any_of(roads.begin(), roads.end(),
                    [this, &new_position](const auto& road) {
                        if (IsRoadContainsPoint(new_position, road)) {
                            dog_->SetPosition(new_position);
                            return true;
                        }
                        return false;
                    })) {
        return MovementInfo{.start_position = dog_->GetPosition(),
                            .end_position = new_position};
    }
    return MoveToBorder(new_position, roads);
}

MovementInfo Player::MoveToBorder(
    const model::Coordinate& new_position,
    const std::vector<model::Road::Pointer>& roads) {
    std::vector<model::Coordinate> candidates;
    candidates.reserve(roads.size());

    for (const auto& road : roads) {
        if (auto bound_pos = FindClampedPositionIfOutside(road, new_position)) {
            candidates.push_back(*bound_pos);
        }
    }

    if (candidates.empty()) {
        return MovementInfo{.start_position = dog_->GetPosition(),
                            .end_position = new_position};
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
    return MovementInfo{.start_position = dog_->GetPosition(),
                        .end_position = final_pos};
}

}  // namespace app
