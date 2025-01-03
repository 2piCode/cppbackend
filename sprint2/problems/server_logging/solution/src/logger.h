#pragma once

#include <boost/date_time.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData",
                            boost::json::value)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

inline void LogFormatter(boost::log::record_view const& rec,
                         boost::log::formatting_ostream& strm) {
    boost::json::object result;
    result["timestamp"] = to_iso_extended_string(*rec[timestamp]);
    result["data"] = *rec[additional_data];
    result["message"] = *rec[boost::log::expressions::smessage];

    strm << boost::json::serialize(result);
}

inline void InitBoostLogFilter() {
    boost::log::add_common_attributes();
    boost::log::add_console_log(std::clog,
                                boost::log::keywords::format = &LogFormatter);
}
