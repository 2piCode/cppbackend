#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
}

TEST_CASE("Text with mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&lt &gt &amp &apos &quot"sv) == "< > & \' \""s);
    CHECK(HtmlDecode("&lt; &gt; &amp; &apos; &quot;"sv) == "< > & \' \""s);
}

TEST_CASE("Text with mnemonics and other symbols", "[HtmlDecode]") {
    CHECK(HtmlDecode("M&amp;M&APOSs"sv) == "M&M's"s);
}

TEST_CASE("Text with different case", "[HtmlDecode]") {
    std::string result = "Johnson&Johnson"s;
    CHECK(HtmlDecode("Johnson&amp;Johnson"sv) == result);
    CHECK(HtmlDecode("Johnson&ampJohnson"sv) == result);
    CHECK(HtmlDecode("Johnson&AMP;Johnson"sv) == result);
    CHECK(HtmlDecode("Johnson&AMPJohnson"sv) == result);
    CHECK(HtmlDecode("Johnson&Johnson"sv) == result);
}

TEST_CASE("Text with mixed case", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&Amp;Johnson"sv) == "Johnson&Amp;Johnson");
    CHECK(HtmlDecode("Johnson&aMp;Johnson"sv) == "Johnson&aMp;Johnson");
    CHECK(HtmlDecode("Johnson&amP;Johnson"sv) == "Johnson&amP;Johnson");
    CHECK(HtmlDecode("Johnson&AMp;Johnson"sv) == "Johnson&AMp;Johnson");
    CHECK(HtmlDecode("Johnson&AmP;Johnson"sv) == "Johnson&AmP;Johnson");
    CHECK(HtmlDecode("Johnson&aMP;Johnson"sv) == "Johnson&aMP;Johnson");
}

TEST_CASE("Text with not ended mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&l &g &am &apo &quo"sv) == "&l &g &am &apo &quo"s);
}

TEST_CASE("Text with connected mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&amp;lt;") == "&lt;"s);
}

// Напишите недостающие тесты самостоятельно
