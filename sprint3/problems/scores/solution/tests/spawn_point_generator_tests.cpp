#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "app/spawn_point_generator.h"
#include "model/map.h"
#include "model/model.h"
#include "model/roads_handler.h"

class MapWithOneRoad : public model::Map {
   public:
    explicit MapWithOneRoad(model::Point center)
        : model::Map(Id{""}, "",
                     Roads{std::make_shared<model::Road>(
                         model::Road::HORIZONTAL, center, 20)},
                     Buildings{}, Offices{}, 0.0, 0) {}
};

SCENARIO("spawnpoint tests") {
    GIVEN("random spawnpoint generator") {
        SpawnPointGenerator generator;

        GIVEN("map with one road") {
            model::Map::Pointer map =
                std::make_unique<MapWithOneRoad>(model::Point{0, 0});
            auto road = map->GetRoads()[0];

            WHEN("generate point") {
                THEN("point must be inside the road") {
                    for (int i = 0; i < 100; i++) {
                        auto point = generator.Generate(map);
                        CHECK(model::IsRoadContainsPoint(point, road));
                    }
                }
            }
        }

        GIVEN("map with reversed map") {
            model::Map::Pointer map =
                std::make_unique<MapWithOneRoad>(model::Point{100, 100});
            auto road = map->GetRoads()[0];

            WHEN("generate point") {
                THEN("point must be inside the road") {
                    for (int i = 0; i < 100; i++) {
                        auto point = generator.Generate(map);
                        CHECK(model::IsRoadContainsPoint(point, road));
                    }
                }
            }
        }
    }

    GIVEN("fixed spawnpoint generator") {
        SpawnPointGenerator generator(false);

        GIVEN("road with start: x=0, y=0") {
            model::Map::Pointer map =
                std::make_unique<MapWithOneRoad>(model::Point{0, 0});

            WHEN("Generate point") {
                auto point = generator.Generate(map);
                THEN("Should return zero point") {
                    CHECK(point.x == 0);
                    CHECK(point.y == 0);
                }
            }
        }

        GIVEN("road with start: x=1, y=1") {
            model::Map::Pointer map =
                std::make_unique<MapWithOneRoad>(model::Point{1, 1});

            WHEN("Generate point") {
                auto point = generator.Generate(map);
                THEN("Should return zero point") {
                    CHECK(point.x == 1);
                    CHECK(point.y == 1);
                }
            }
        }

        GIVEN("road with start: x=544 y=765") {
            model::Map::Pointer map =
                std::make_unique<MapWithOneRoad>(model::Point{544, 765});

            WHEN("Generate point") {
                auto point = generator.Generate(map);
                THEN("Should return zero point") {
                    CHECK(point.x == 544);
                    CHECK(point.y == 765);
                }
            }
        }
    }
}
