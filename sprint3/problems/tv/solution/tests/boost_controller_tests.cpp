#include <iostream>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include "../src/controller.h"
#include "boost_test_helpers.h"

using namespace std::literals;

struct ControllerFixture {
    TV tv;
    std::istringstream input;
    std::ostringstream output;
    Menu menu{input, output};
    Controller controller{tv, menu};

    void RunMenuCommand(std::string command) {
        input.str(std::move(command));
        input.clear();
        menu.Run();
    }

    void ExpectExtraArgumentsErrorInOutput(std::string_view command) const {
        ExpectOutput("Error: the "s.append(command).append(
            " command does not require any arguments\n"sv));
    }

    void ExpectEmptyOutput() const { ExpectOutput({}); }

    void ExpectOutput(std::string_view expected) const {
        // В g++ 10.3 не реализован метод ostringstream::view(), поэтому
        // приходится использовать метод str()
        BOOST_TEST(output.str() == expected);
    }
};

struct WhenTVIsOffFixture : ControllerFixture {
    WhenTVIsOffFixture() { BOOST_REQUIRE(!tv.IsTurnedOn()); }
};

BOOST_AUTO_TEST_SUITE(Controller_)

BOOST_FIXTURE_TEST_SUITE(WhenTVIsOff, WhenTVIsOffFixture)
BOOST_AUTO_TEST_CASE(on_Info_command_prints_that_tv_is_off) {
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned off\n"sv);
    BOOST_TEST(!tv.IsTurnedOn());
}
BOOST_AUTO_TEST_CASE(
    on_Info_command_prints_error_message_if_comand_has_any_args) {
    RunMenuCommand("Info some extra args"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("Info"sv);
}
BOOST_AUTO_TEST_CASE(on_Info_command_ignores_trailing_spaces) {
    RunMenuCommand("Info  "s);
    ExpectOutput("TV is turned off\n"sv);
}
BOOST_AUTO_TEST_CASE(on_TurnOn_command_turns_TV_on) {
    RunMenuCommand("TurnOn"s);
    BOOST_TEST(tv.IsTurnedOn());
    ExpectEmptyOutput();
}
BOOST_AUTO_TEST_CASE(
    on_TurnOn_command_with_some_arguments_prints_error_message) {
    RunMenuCommand("TurnOn some args"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOn"sv);
}
BOOST_AUTO_TEST_CASE(on_Select_Channel_without_channel_number) {
    RunMenuCommand("SelectChannel"s);
    ExpectOutput(
        "Error: the SelectChannel command requires channel number argument\n"sv);
}
BOOST_AUTO_TEST_CASE(on_Select_Channel_with_turned_off_tv) {
    RunMenuCommand("SelectChannel 1"s);
    ExpectOutput("TV is turned off\n"sv);
}
BOOST_AUTO_TEST_CASE(on_Select_Previous_Channel_with_extra_argument) {
    RunMenuCommand("SelectPreviousChannel some extra args"s);
    ExpectExtraArgumentsErrorInOutput("SelectPreviousChannel"sv);
}
BOOST_AUTO_TEST_CASE(on_Select_Previous_Channel_with_turned_off_tv) {
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectOutput("TV is turned off\n"sv);
}
BOOST_AUTO_TEST_SUITE_END()

struct WhenTVIsOnFixture : ControllerFixture {
    WhenTVIsOnFixture() { tv.TurnOn(); }
};

BOOST_FIXTURE_TEST_SUITE(WhenTVIsOn, WhenTVIsOnFixture)
BOOST_AUTO_TEST_CASE(on_TurnOff_command_turns_tv_off) {
    RunMenuCommand("TurnOff"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectEmptyOutput();
}
BOOST_AUTO_TEST_CASE(
    on_TurnOff_command_with_some_arguments_prints_error_message) {
    RunMenuCommand("TurnOff some args"s);
    BOOST_TEST(tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOff"sv);
}
BOOST_AUTO_TEST_CASE(on_Info_prints_current_channel) {
    tv.SelectChannel(42);
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned on\nChannel number is 42\n"sv);
}
BOOST_AUTO_TEST_CASE(on_Select_Channel_with_invalid_channel_str) {
    RunMenuCommand("SelectChannel first"s);
    ExpectOutput("Invalid channel\n"sv);
}
BOOST_AUTO_TEST_CASE(on_Select_Channel_with_invalid_channel_number) {
    RunMenuCommand("SelectChannel 0"s);
    RunMenuCommand("SelectChannel 100"s);
    ExpectOutput("Channel is out of range\nChannel is out of range\n"sv);
}
BOOST_AUTO_TEST_CASE(on_Select_Channel_with_valid_channel_number) {
    RunMenuCommand("SelectChannel 10"s);
    ExpectEmptyOutput();
    BOOST_TEST(tv.GetChannel() == 10);
}
BOOST_AUTO_TEST_CASE(on_Select_Previous_Channel_with_turned_on_tv) {
    RunMenuCommand("SelectChannel 10"s);
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectEmptyOutput();
    BOOST_TEST(tv.GetChannel() == 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
