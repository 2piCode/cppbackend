#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tagged.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Coordinate {
    double x, y;
};

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

   public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}, end_{end_x, start.y} {}

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}, end_{start.x, end_y} {}

    bool IsHorizontal() const noexcept { return start_.y == end_.y; }

    bool IsVertical() const noexcept { return start_.x == end_.x; }

    Point GetStart() const noexcept { return start_; }

    Point GetEnd() const noexcept { return end_; }

   private:
    Point start_;
    Point end_;
};

class Building {
   public:
    explicit Building(Rectangle bounds) noexcept : bounds_{bounds} {}

    const Rectangle& GetBounds() const noexcept { return bounds_; }

   private:
    Rectangle bounds_;
};

class Office {
   public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}, position_{position}, offset_{offset} {}

    const Id& GetId() const noexcept { return id_; }

    Point GetPosition() const noexcept { return position_; }

    Offset GetOffset() const noexcept { return offset_; }

   private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
   public:
    using Id = util::Tagged<std::string, Map>;
    using Pointer = std::shared_ptr<Map>;
    using ConstPointer = const std::shared_ptr<Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name, std::optional<double> dog_speed) noexcept
        : id_(std::move(id)), name_(std::move(name)), dog_speed_(dog_speed) {}

    const Id& GetId() const noexcept { return id_; }

    const std::string& GetName() const noexcept { return name_; }

    const Buildings& GetBuildings() const noexcept { return buildings_; }

    const Roads& GetRoads() const noexcept { return roads_; }

    const Offices& GetOffices() const noexcept { return offices_; }

    void AddRoad(const Road& road) { roads_.emplace_back(road); }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

    std::optional<double> GetMaxSpeed() const noexcept { return dog_speed_; }

   private:
    using OfficeIdToIndex =
        std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;
    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
    std::optional<double> dog_speed_;
};

enum class Direction { NORTH, SOUTH, WEST, EAST, NONE };

namespace direction_converter {

inline std::string ToString(Direction direction) {
    static std::unordered_map<model::Direction, std::string> map{
        {Direction::NORTH, "U"},
        {Direction::SOUTH, "D"},
        {Direction::EAST, "R"},
        {Direction::WEST, "L"},
        {Direction::NONE, ""}};
    return map[direction];
}

inline Direction ToDirection(std::string direction) {
    static std::unordered_map<std::string, model::Direction> map{
        {"U", Direction::NORTH},
        {"D", Direction::SOUTH},
        {"R", Direction::EAST},
        {"L", Direction::WEST},
        {"", Direction::NONE}};

    if (auto it = map.find(direction); it != map.end()) {
        return it->second;
    }

    throw std::invalid_argument("Invalid direction");
}

}  // namespace direction_converter

class Dog {
   public:
    using Id = util::Tagged<std::uint32_t, Dog>;
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
                velocity_per_second_ = {0, max_speed_};
                break;
            case Direction::SOUTH:
                velocity_per_second_ = {0, -max_speed_};
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

   private:
    const Id id_;
    const std::string name_;
    const double max_speed_;

    Coordinate position_;
    Coordinate velocity_per_second_ = {0, 0};
    Direction direction_ = Direction::NORTH;
};

}  // namespace model
