#include <catch2/catch_test_macros.hpp>

#include "../src/tv.h"

namespace Catch {

template <>
struct StringMaker<std::nullopt_t> {
    static std::string convert(std::nullopt_t) {
        using namespace std::literals;
        return "nullopt"s;
    }
};

template <typename T>
struct StringMaker<std::optional<T>> {
    static std::string convert(const std::optional<T>& opt_value) {
        if (opt_value) {
            return StringMaker<T>::convert(*opt_value);
        } else {
            return StringMaker<std::nullopt_t>::convert(std::nullopt);
        }
    }
};

}  // namespace Catch

SCENARIO("TV", "[TV]") {
    GIVEN("A TV") {  // Дано: Телевизор
        TV tv;

        // Изначально он выключен и не показывает никаких каналов
        SECTION("Initially it is off and doesn't show any channel") {
            CHECK(!tv.IsTurnedOn());
            CHECK(!tv.GetChannel().has_value());
        }

        // Когда он выключен,
        WHEN("it is turned off") {
            REQUIRE(!tv.IsTurnedOn());

            // он не может переключать каналы
            THEN("it can't select any channel") {
                CHECK_THROWS_AS(tv.SelectChannel(10), std::logic_error);
                CHECK(tv.GetChannel() == std::nullopt);
                tv.TurnOn();
                CHECK(tv.GetChannel() == 1);
            }

            THEN("it can't select previous channel") {
                CHECK_THROWS_AS(tv.SelectLastViewedChannel(), std::logic_error);
                tv.TurnOn();
                tv.SelectLastViewedChannel();
                CHECK(tv.GetChannel() == 1);
            }
        }

        WHEN(
            "it is turned on first time") {  // Когда его включают в первый раз,
            tv.TurnOn();

            // то он включается и показывает канал #1
            THEN("it is turned on and shows channel #1") {
                CHECK(tv.IsTurnedOn());
                CHECK(tv.GetChannel() == 1);

                // А когда его выключают,
                AND_WHEN("it is turned off") {
                    tv.TurnOff();

                    // то он выключается и не показывает никаких каналов
                    THEN("it is turned off and doesn't show any channel") {
                        CHECK(!tv.IsTurnedOn());
                        CHECK(tv.GetChannel() == std::nullopt);
                    }
                }
            }
            // И затем может выбирать канал с 1 по 99
            AND_THEN("it can select channel from MIN_CHANNEl to MAX_CHANNEL") {
                for (int i = tv.MIN_CHANNEL; i < tv.MAX_CHANNEL; i++) {
                    tv.SelectChannel(i);
                    CHECK(tv.GetChannel() == i);
                }
            }

            AND_THEN(
                "it can't select channel less than MIN_CHANNEL or more than "
                "MAX_CHANNEL") {
                CHECK_THROWS_AS(tv.SelectChannel(tv.MIN_CHANNEL - 1),
                                std::out_of_range);
                CHECK_THROWS_AS(tv.SelectChannel(tv.MAX_CHANNEL + 1),
                                std::out_of_range);
            }

            AND_THEN("it can select previous channel after set new channel") {
                auto channel = tv.GetChannel();
                tv.SelectChannel(5);
                tv.SelectLastViewedChannel();
                CHECK(tv.GetChannel() == channel);
            }

            AND_THEN(
                "it can select previous channel after selected previous "
                "channel") {
                int channel = 5;
                tv.SelectChannel(channel);
                tv.SelectLastViewedChannel();
                tv.SelectLastViewedChannel();
                CHECK(tv.GetChannel() == channel);
            }

            AND_THEN(
                "it can select last viewed channel after twice set same "
                "channel") {
                auto channel = tv.GetChannel();
                tv.SelectChannel(5);
                tv.SelectChannel(5);
                tv.SelectLastViewedChannel();
                CHECK(tv.GetChannel() == channel);
            }
        }
    }
}
