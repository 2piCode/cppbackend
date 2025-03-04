#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    ASSERT_EQ(UrlEncode("hello"sv), "hello"s);
}

TEST(UrlEncodeTestSuite, EmptyString) { ASSERT_EQ(UrlEncode(""sv), ""s); }

TEST(UrlEncodeTestSuite, SymbolsAreEncoded) {
    ASSERT_EQ(UrlEncode("!#$&'()*+,/:;=?@[]"sv),
              "%21%23%24%26%27%28%29%2a%2b%2c%2f%3a%3b%3d%3f%40%5b%5d"s);
}

TEST(UrlEncodeTestSuite, SpacesAndSpecialCasesAreEncoded) {
    EXPECT_EQ(UrlEncode(" "), "+");
    EXPECT_EQ(UrlEncode("  "), "++");
    EXPECT_EQ(UrlEncode("\n"), "%0a");
    EXPECT_EQ(UrlEncode("\t"), "%09");
}

TEST(UrlEncodeTestSuite, BaseExample) {
    ASSERT_EQ(UrlEncode("Hello World !"), "Hello+World+%21");
    ASSERT_EQ(UrlEncode("abc*"sv), "abc%2a");
}

/* Напишите остальные тесты самостоятельно */
