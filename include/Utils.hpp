#pragma once

#include <boost/json.hpp>

#include <string>

namespace json = boost::json;

namespace utils {

template <typename T> T getValue(const json::value& val);

template <> std::string getValue<std::string>(const json::value& val);

template <> int64_t getValue<int64_t>(const json::value& val);

template <> double getValue<double>(const json::value& val);

template <> bool getValue<bool>(const json::value& val);

template <typename T> T getFieldValue(const json::object& json_obj, const std::string& field_name);

std::string generateUuid();

} // namespace utils
