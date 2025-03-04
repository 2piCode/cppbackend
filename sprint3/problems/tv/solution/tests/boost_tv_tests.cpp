#include <boost/throw_exception.hpp>
#define BOOST_TEST_MODULE TV tests
#include <boost/test/unit_test.hpp>

#include "../src/tv.h"
#include "boost_test_helpers.h"

struct TVFixture {
    TV tv;
};
BOOST_FIXTURE_TEST_SUITE(TV_, TVFixture)
BOOST_AUTO_TEST_CASE(is_off_by_default) {
    // Внутри теста поля структуры TVFixture доступны по их имени
    BOOST_TEST(!tv.IsTurnedOn());
}
BOOST_AUTO_TEST_CASE(doesnt_show_any_channel_by_default) {
    BOOST_TEST(!tv.GetChannel().has_value());
}

BOOST_AUTO_TEST_CASE(cant_select_any_channel_when_it_is_off) {
    BOOST_CHECK_THROW(tv.SelectChannel(10), std::logic_error);
    BOOST_TEST(tv.GetChannel() == std::nullopt);
    tv.TurnOn();
    BOOST_TEST(tv.GetChannel() == 1);
}

BOOST_AUTO_TEST_CASE(can_not_select_previous_chanel_when_is_is_off) {
    BOOST_CHECK_THROW(tv.SelectLastViewedChannel(), std::logic_error);
    tv.TurnOn();
    tv.SelectLastViewedChannel();
    BOOST_CHECK(tv.GetChannel() == 1);
}

// Тестовый стенд "Включенный телевизор" унаследован от TVFixture.
struct TurnedOnTVFixture : TVFixture {
    // В конструкторе выполняем донастройку унаследованного поля tv
    TurnedOnTVFixture() { tv.TurnOn(); }
};
// (Телевизор) после включения
BOOST_FIXTURE_TEST_SUITE(After_turning_on_, TurnedOnTVFixture)
// показывает канал #1
BOOST_AUTO_TEST_CASE(shows_channel_1) {
    BOOST_TEST(tv.IsTurnedOn());
    BOOST_TEST(tv.GetChannel() == 1);
}
// Может быть выключен
BOOST_AUTO_TEST_CASE(can_be_turned_off) {
    tv.TurnOff();
    BOOST_TEST(!tv.IsTurnedOn());
    BOOST_TEST(tv.GetChannel() == std::nullopt);
}
// Может выбирать каналы с 1 по 99
BOOST_AUTO_TEST_CASE(can_select_channel_from_min_to_max) {
    for (int i = tv.MIN_CHANNEL; i <= tv.MAX_CHANNEL; i++) {
        tv.SelectChannel(i);
        BOOST_TEST(tv.GetChannel() == i);
    }
}

BOOST_AUTO_TEST_CASE(can_not_select_channel_less_than_min_or_more_than_max) {
    BOOST_CHECK_THROW(tv.SelectChannel(tv.MIN_CHANNEL - 1), std::out_of_range);
    BOOST_CHECK_THROW(tv.SelectChannel(tv.MAX_CHANNEL + 1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(can_select_previous_channel_after_set_new_channel) {
    auto channel = tv.GetChannel();

    tv.SelectChannel(5);
    tv.SelectLastViewedChannel();
    BOOST_TEST(tv.GetChannel() == channel);
}

BOOST_AUTO_TEST_CASE(
    can_select_previous_channel_after_selected_previous_channel) {
    int channel = 5;
    tv.SelectChannel(channel);
    tv.SelectLastViewedChannel();
    tv.SelectLastViewedChannel();
    BOOST_TEST(tv.GetChannel() == channel);
}

BOOST_AUTO_TEST_CASE(can_select_previous_channel_after_twice_set_same_channel) {
    auto channel = tv.GetChannel();
    tv.SelectChannel(5);
    tv.SelectChannel(5);
    tv.SelectLastViewedChannel();
    BOOST_TEST(tv.GetChannel() == channel);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
