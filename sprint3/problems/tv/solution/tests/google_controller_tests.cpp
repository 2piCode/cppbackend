#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "../src/controller.h"

using namespace std::literals;

class ControllerWithTurnedOffTV : public testing::Test {
   protected:
    void SetUp() override { ASSERT_FALSE(tv_.IsTurnedOn()); }

    void RunMenuCommand(std::string command) {
        input_.str(std::move(command));
        input_.clear();
        menu_.Run();
    }

    void ExpectExtraArgumentsErrorInOutput(std::string_view command) const {
        ExpectOutput("Error: the "s.append(command).append(
            " command does not require any arguments\n"sv));
    }

    void ExpectEmptyOutput() const { ExpectOutput({}); }

    void ExpectOutput(std::string_view expected) const {
        // В g++ 10.3 не реализован метод ostringstream::view(), поэтому
        // приходится использовать метод str()
        EXPECT_EQ(output_.str(), std::string{expected});
    }

    TV tv_;
    std::istringstream input_;
    std::ostringstream output_;
    Menu menu_{input_, output_};
    Controller controller_{tv_, menu_};
};

TEST_F(ControllerWithTurnedOffTV, OnInfoCommandPrintsThatTVIsOff) {
    input_.str("Info"s);
    menu_.Run();
    ExpectOutput("TV is turned off\n"sv);
    EXPECT_FALSE(tv_.IsTurnedOn());
}
TEST_F(ControllerWithTurnedOffTV,
       OnInfoCommandPrintsErrorMessageIfCommandHasAnyArgs) {
    RunMenuCommand("Info some extra args"s);
    EXPECT_FALSE(tv_.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("Info"sv);
}
TEST_F(ControllerWithTurnedOffTV,
       OnInfoCommandWithTrailingSpacesPrintsThatTVIsOff) {
    input_.str("Info  "s);
    menu_.Run();
    ExpectOutput("TV is turned off\n"sv);
}
TEST_F(ControllerWithTurnedOffTV, OnTurnOnCommandTurnsTVOn) {
    RunMenuCommand("TurnOn"s);
    EXPECT_TRUE(tv_.IsTurnedOn());
    ExpectEmptyOutput();
}
TEST_F(ControllerWithTurnedOffTV,
       OnTurnOnCommandPrintsErrorMessageIfCommandHasAnyArgs) {
    RunMenuCommand("TurnOn some extra args"s);
    EXPECT_FALSE(tv_.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOn"sv);
}
TEST_F(ControllerWithTurnedOffTV, OnSelectChannelWithoutChannelNumber) {
    RunMenuCommand("SelectChannel"s);
    ExpectOutput(
        "Error: the SelectChannel command requires channel number argument\n"sv);
}
TEST_F(ControllerWithTurnedOffTV, OnSelectChannelWithTurnedOffTV) {
    RunMenuCommand("SelectChannel 1"s);
    ExpectOutput("TV is turned off\n"sv);
}
TEST_F(ControllerWithTurnedOffTV, OnSelectPreviousChannelWithExtraArgument) {
    RunMenuCommand("SelectPreviousChannel some extra args"s);
    ExpectExtraArgumentsErrorInOutput("SelectPreviousChannel"sv);
}
TEST_F(ControllerWithTurnedOffTV, OnSelectPreviousChannelWithTurnedOffTV) {
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectOutput("TV is turned off\n"sv);
}

//-----------------------------------------------------------------------------------

class ControllerWithTurnedOnTV : public ControllerWithTurnedOffTV {
   protected:
    void SetUp() override { tv_.TurnOn(); }
};

TEST_F(ControllerWithTurnedOnTV, OnTurnOffCommandTurnsTVOff) {
    RunMenuCommand("TurnOff"s);
    EXPECT_FALSE(tv_.IsTurnedOn());
    ExpectEmptyOutput();
}
TEST_F(ControllerWithTurnedOnTV,
       OnTurnOffCommandPrintsErrorMessageIfCommandHasAnyArgs) {
    RunMenuCommand("TurnOff some extra args"s);
    EXPECT_TRUE(tv_.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOff"sv);
}
TEST_F(ControllerWithTurnedOnTV, OnInfoPrintsCurrentChannel) {
    tv_.SelectChannel(42);
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned on\nChannel number is 42\n"sv);
}
TEST_F(ControllerWithTurnedOnTV, OnSelectChannelWithInvalidChannelStr) {
    RunMenuCommand("SelectChannel first"s);
    ExpectOutput("Invalid channel\n"sv);
}
TEST_F(ControllerWithTurnedOnTV, OnSelectChannelWithInvalidChannelNumber) {
    RunMenuCommand("SelectChannel 0"s);
    RunMenuCommand("SelectChannel 100"s);
    ExpectOutput("Channel is out of range\nChannel is out of range\n"sv);
}
TEST_F(ControllerWithTurnedOnTV, OnSelectChannelWithValidChannelNumber) {
    RunMenuCommand("SelectChannel 10"s);
    ExpectEmptyOutput();
    EXPECT_EQ(tv_.GetChannel(), 10);
}
TEST_F(ControllerWithTurnedOnTV, OnSelectPreviousChannelWithTurnedOnTV) {
    RunMenuCommand("SelectChannel 10"s);
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectEmptyOutput();
    EXPECT_EQ(tv_.GetChannel(), 1);
}
