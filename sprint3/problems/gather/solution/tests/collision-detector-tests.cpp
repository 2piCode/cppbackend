#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <csignal>
#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>

using namespace std::literals;

#include <sstream>
#include <vector>

#include "../src/collision_detector.h"

using namespace Catch::Matchers;

namespace Catch {
template <>
struct StringMaker<collision_detector::GatheringEvent> {
    static std::string convert(
        collision_detector::GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(" << value.gatherer_id << "," << value.item_id << ","
            << value.sq_distance << "," << value.time << ")";

        return tmp.str();
    }
};
}  // namespace Catch

class TestProvider : public collision_detector::ItemGathererProvider {
   public:
    size_t ItemsCount() const override { return items.size(); }

    size_t GatherersCount() const override { return gatherers.size(); }

    collision_detector::Item GetItem(size_t idx) const override {
        return items.at(idx);
    }

    collision_detector::Gatherer GetGatherer(size_t idx) const override {
        return gatherers.at(idx);
    }

    std::vector<collision_detector::Item> items;
    std::vector<collision_detector::Gatherer> gatherers;
};

bool IsSortedByTime(
    const std::vector<collision_detector::GatheringEvent>& events) {
    for (size_t i = 1; i < events.size(); ++i) {
        if (events[i].time < events[i - 1].time) {
            return false;
        }
    }
    return true;
}

SCENARIO("Test for find gather events function"s) {
    TestProvider provider;

    WHEN("empty items and gatherers") {
        auto result = collision_detector::FindGatherEvents(provider);
        THEN("return empty result") { CHECK(result.empty()); }
    }

    GIVEN("item with coord x = 1.0 y = 1.2 and width = 0.5") {
        provider.items.push_back(collision_detector::Item{{1.0, 1.2}, 0.5});

        AND_GIVEN("gatherer with same start and end points") {
            collision_detector::Gatherer gatherer{{0.0, 0.0}, {0.0, 0.0}, 0.6};
            provider.gatherers.push_back(gatherer);
            THEN("return empty result") {
                CHECK(collision_detector::FindGatherEvents(provider).empty());
            }
        }

        AND_GIVEN("gather with path included item") {
            collision_detector::Gatherer gatherer{
                {0.0, 0.0}, {10.0, 10.0}, 0.6};
            provider.gatherers.push_back(gatherer);

            THEN("return correct item") {
                auto events = collision_detector::FindGatherEvents(provider);

                REQUIRE_THAT(events, SizeIs(1));
                CHECK(events[0].item_id == 0);
                CHECK_THAT(events[0].sq_distance, WithinAbs(0.02, 1e-6));
                CHECK(events[0].time == 0.11);
            }
        }

        AND_GIVEN("gather width end point in x = 2.0 y = 0.0 and width = 1.0") {
            provider.gatherers.push_back(
                collision_detector::Gatherer{{0, 0}, {2, 0}, 1.0});

            THEN("should include point by width") {
                REQUIRE_THAT(FindGatherEvents(provider), SizeIs(1));
            }
        }
    }

    GIVEN("points with coords: {5, 0} {0, 5} {1, 1}") {
        provider.items.push_back(collision_detector::Item{{5, 0}, 0.5});
        provider.items.push_back(collision_detector::Item{{0, 5}, 0.5});
        provider.items.push_back(collision_detector::Item{{1, 1}, 0.5});

        AND_GIVEN("providers width different dirrection") {
            provider.gatherers.push_back(
                collision_detector::Gatherer{{0, 0}, {10, 0}, 1.0});
            provider.gatherers.push_back(
                collision_detector::Gatherer{{0, 0}, {0, 10}, 1.0});
            THEN("should return sorted collection by time") {
                auto correct_events = FindGatherEvents(provider);
                REQUIRE_THAT(correct_events, SizeIs(4));
                CHECK(IsSortedByTime(correct_events));
            }
        }
    }

    GIVEN("gather with start point {0, 0}, endpoint {10, 10} and width = 1.0") {
        provider.gatherers.push_back(
            collision_detector::Gatherer{{0, 0}, {10, 10}, 1.0});

        AND_GIVEN("item width coord x = 5.0 y = 6.0 and width = 1.0") {
            provider.items.push_back(collision_detector::Item{{5.0, 6.0}, 0.5});

            THEN("should find point by diagonal path") {
                auto events = FindGatherEvents(provider);
                REQUIRE_THAT(events, SizeIs(1));
                CHECK(events[0].item_id == 0);
                CHECK(events[0].gatherer_id == 0);
                CHECK_THAT(events[0].time, WithinAbs(0.55, 1e-6));
            }
        }
    }
}
