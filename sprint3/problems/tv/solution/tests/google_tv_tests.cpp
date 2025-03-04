#include <cinttypes>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <stdexcept>

#include "../src/tv.h"

// Тестовый стенд "Телевизор по умолчанию"
class TVByDefault : public testing::Test {
   protected:
    TV tv_;
};
TEST_F(TVByDefault, IsOff) { EXPECT_FALSE(tv_.IsTurnedOn()); }

TEST_F(TVByDefault, DoesntShowAChannelWhenItIsOff) {
    EXPECT_FALSE(tv_.GetChannel().has_value());
}

TEST_F(TVByDefault, CantSelectAnyChannel) {
    EXPECT_THROW(tv_.SelectChannel(10), std::logic_error);
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
    tv_.TurnOn();
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}

TEST_F(TVByDefault, CanNotSelectPreviousChannelWhenItIsOff) {
    EXPECT_THROW(tv_.SelectLastViewedChannel(), std::logic_error);
    tv_.TurnOn();
    tv_.SelectLastViewedChannel();
    EXPECT_EQ(tv_.GetChannel(), 1);
}

// Тестовый стенд "Включенный телевизор"
class TurnedOnTV : public TVByDefault {
   protected:
    void SetUp() override { tv_.TurnOn(); }
};
TEST_F(TurnedOnTV, ShowsChannel1) {
    EXPECT_TRUE(tv_.IsTurnedOn());
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}
TEST_F(TurnedOnTV, AfterTurningOffTurnsOffAndDoesntShowAnyChannel) {
    tv_.TurnOff();
    EXPECT_FALSE(tv_.IsTurnedOn());
    // Сравнение с nullopt в GoogleTest выполняется так:
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
}
TEST_F(TurnedOnTV, CanSelectChannelFromMinToMax) {
    for (int i = tv_.MIN_CHANNEL; i <= tv_.MAX_CHANNEL; i++) {
        tv_.SelectChannel(i);
        EXPECT_EQ(tv_.GetChannel(), i);
    }
}
TEST_F(TurnedOnTV, CanNotSelectChannelLessThanMinOrMoreThanMaxNumberOfChannel) {
    EXPECT_THROW(tv_.SelectChannel(tv_.MIN_CHANNEL - 1), std::out_of_range);
    EXPECT_THROW(tv_.SelectChannel(tv_.MAX_CHANNEL + 1), std::out_of_range);
}
TEST_F(TurnedOnTV, CanSelectPreviousChannelAfterSetNewChannel) {
    auto channel = tv_.GetChannel();

    tv_.SelectChannel(5);
    tv_.SelectLastViewedChannel();
    EXPECT_EQ(tv_.GetChannel(), channel);
}
TEST_F(TurnedOnTV, CanSelectPreviousChannelAfterSelectedPreviousChannel) {
    int channel = 5;

    tv_.SelectChannel(channel);
    tv_.SelectLastViewedChannel();
    tv_.SelectLastViewedChannel();
    EXPECT_EQ(tv_.GetChannel(), channel);
}
TEST_F(TurnedOnTV, CanSelectPreviousChannelAfterTwiceSetSameChannel) {
    auto channel = tv_.GetChannel();
    tv_.SelectChannel(5);
    tv_.SelectChannel(5);
    tv_.SelectLastViewedChannel();
    EXPECT_EQ(tv_.GetChannel(), channel);
}
