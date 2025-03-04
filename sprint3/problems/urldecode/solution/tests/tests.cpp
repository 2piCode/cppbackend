#include <stdexcept>

#define BOOST_TEST_MODULE urlencode tests

#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);
    BOOST_TEST(UrlDecode("Hello, world!"sv) == "Hello, world!"s);
    BOOST_TEST(UrlDecode("Hello%2C%20world!"sv) == "Hello, world!"s);
    BOOST_TEST(UrlDecode("Hello%2c%20world!"sv) == "Hello, world!"s);
    BOOST_CHECK_THROW(UrlDecode("Hello%2world!"sv), std::invalid_argument);
    BOOST_TEST(UrlDecode("Hello%2C+world!"sv) == "Hello, world!"s);
}
