#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "tagged.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Coordinate {
    double x, y;

    Coordinate operator+(const Coordinate& other) const {
        return {x + other.x, y + other.y};
    }

    Coordinate operator*(double scalar) const {
        return {x * scalar, y * scalar};
    }
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
    using Pointer = std::shared_ptr<Road>;

    constexpr static double WIDTH = 0.4;

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

}  // namespace model
