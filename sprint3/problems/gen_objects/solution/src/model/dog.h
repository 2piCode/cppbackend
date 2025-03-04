#pragma once

#include <string>

#include "model.h"

namespace model {

class Dog {
   public:
    using Id = util::Tagged<std::uint32_t, Dog>;
    using Pointer = Dog*;
    using ConstPointer = const Dog*;

    explicit Dog(std::uint32_t id, std::string name, double max_speed,
                 Coordinate position)
        : id_{id}, name_{name}, max_speed_(max_speed), position_(position) {}

    Id GetId() const noexcept { return id_; }
    std::string_view GetName() const noexcept { return name_; }
    Coordinate GetPosition() const noexcept { return position_; }
    Coordinate GetVelocity() const noexcept { return velocity_per_second_; }
    Direction GetDirection() const noexcept { return direction_; }

    void SetDirection(Direction direction) {
        direction_ = direction;
        switch (direction) {
            case Direction::NORTH:
                velocity_per_second_ = {0, -max_speed_};
                break;
            case Direction::SOUTH:
                velocity_per_second_ = {0, max_speed_};
                break;
            case Direction::EAST:
                velocity_per_second_ = {max_speed_, 0};
                break;
            case Direction::WEST:
                velocity_per_second_ = {-max_speed_, 0};
                break;
            case Direction::NONE:
                velocity_per_second_ = {0, 0};
                break;
        }
    }

    void SetPosition(Coordinate position) { position_ = position; }
    void Stop() { velocity_per_second_ = {0, 0}; }

   private:
    const Id id_;
    const std::string name_;
    const double max_speed_;

    Coordinate position_;
    Coordinate velocity_per_second_ = {0, 0};
    Direction direction_ = Direction::NORTH;
};

}  // namespace model
