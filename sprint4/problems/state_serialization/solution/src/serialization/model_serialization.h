#pragma once

#include <string>
#include <vector>

#include <boost/serialization/vector.hpp>

#include "model/dog.h"
#include "model/item.h"
#include "model/model.h"

namespace model {

template <typename Archive>
void serialize(Archive& ar, Point& point,
               [[maybe_unused]] const unsigned version) {
    ar & point.x;
    ar & point.y;
}

template <typename Archive>
void serialize(Archive& ar, Size& size,
               [[maybe_unused]] const unsigned version) {
    ar & size.width;
    ar & size.height;
}

template <typename Archive>
void serialize(Archive& ar, Coordinate& coord,
               [[maybe_unused]] const unsigned version) {
    ar & coord.x;
    ar & coord.y;
}

template <typename Archive>
void serialize(Archive& ar, model::Rectangle& rectangle,
               [[maybe_unused]] const unsigned version) {
    ar & rectangle.position;
    ar & rectangle.size;
}

template <typename Archive>
void serialize(Archive& ar, model::Offset& offset,
               [[maybe_unused]] const unsigned version) {
    ar & offset.dx;
    ar & offset.dy;
}

template <typename Archive>
void serialize(Archive& ar, Item::Id& id,
               [[maybe_unused]] const unsigned version) {
    ar&* id;
}

template <typename Archive>
void serialize(Archive& ar, Item& item,
               [[maybe_unused]] const unsigned version) {
    ar & item.id;
    ar & item.type;
    ar & item.value;
    ar & item.position;
}

}  // namespace model

namespace serialization {

class DogRepr {
   public:
    using Id = model::Dog::Id;
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
        : id_(dog.id_),
          name_(dog.name_),
          max_speed_(dog.max_speed_),
          items_(dog.items_),
          scores_(dog.scores_),
          position_(dog.position_),
          direction_(dog.direction_),
          velocity_per_second_(dog.velocity_per_second_) {}

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* id_;
        ar & name_;
        ar & max_speed_;
        ar & items_;
        ar & scores_;
        ar & position_;
        ar & direction_;
        ar & velocity_per_second_;
    }

    [[nodiscard]] model::Dog Restore() const {
        model::Dog dog(*id_, name_, max_speed_, position_);
        dog.items_ = items_;
        dog.scores_ = scores_;
        dog.direction_ = direction_;
        dog.velocity_per_second_ = velocity_per_second_;
        return dog;
    }

   private:
    Id id_ = Id{0u};
    std::string name_;
    double max_speed_ = 0.0;
    std::vector<model::Item> items_;
    int scores_ = 0;

    model::Coordinate position_;
    model::Coordinate velocity_per_second_ = {0, 0};
    model::Direction direction_ = model::Direction::NORTH;
};

}  // namespace serialization
