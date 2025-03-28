#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "item.h"
#include "model.h"

namespace serialization {
class DogRepr;
}

namespace model {

class Dog {
   public:
    friend class serialization::DogRepr;
    using Id = util::Tagged<std::uint32_t, Dog>;
    using Pointer = Dog*;
    using ConstPointer = const Dog*;

    explicit Dog(uint32_t id, std::string name, double max_speed,
                 Coordinate position)
        : id_{id}, name_{name}, max_speed_(max_speed), position_(position) {}

    Dog(const Dog& other) = default;
    Dog& operator=(const Dog& other) = default;
    Dog(Dog&& other) = default;
    Dog& operator=(Dog&& other) = default;

    Id GetId() const noexcept { return id_; }
    std::string_view GetName() const noexcept { return name_; }
    Coordinate GetPosition() const noexcept { return position_; }
    Coordinate GetVelocity() const noexcept { return velocity_per_second_; }
    Direction GetDirection() const noexcept { return direction_; }
    double GetMaxSpeed() const noexcept { return max_speed_; }

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

    void AddItem(Item item) { items_.push_back(item); }

    std::vector<Item> DropAllItems() {
        IncreaseScore();
        std::vector<Item> items = items_;
        items_.clear();
        return items;
    }

    size_t GetItemCount() const { return items_.size(); }
    std::vector<Item> GetItems() const { return items_; }
    int GetScore() const { return scores_; }

    void SetTimeInGame(std::chrono::milliseconds time_in_game) {
        time_in_game_ = time_in_game;
    }

    void SetLastMoveTime(std::chrono::milliseconds last_move_time) {
        last_move_time_ = last_move_time;
    }

    std::chrono::milliseconds GetTimeInGame() const { return time_in_game_; }
    std::chrono::milliseconds GetLastMoveTime() const {
        return last_move_time_;
    }

   private:
    Id id_;
    std::string name_;
    double max_speed_;
    std::vector<Item> items_;
    int scores_ = 0;

    Coordinate position_;
    Coordinate velocity_per_second_ = {0, 0};
    Direction direction_ = Direction::NORTH;

    std::chrono::milliseconds time_in_game_ = std::chrono::milliseconds(0);
    std::chrono::milliseconds last_move_time_ = std::chrono::milliseconds(0);

    void IncreaseScore() {
        for (const auto& item : items_) {
            scores_ += item.value;
        }
    }
};

}  // namespace model
