#include "Utils.hpp"
#include <stdexcept>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

namespace utils {

template <> std::string getValue<std::string>(const json::value& val) {
    return val.as_string().c_str();
}

template <> uint64_t getValue<uint64_t>(const json::value& val) {
    return val.as_int64();
}

template <> double getValue<double>(const json::value& val) {
    return val.as_double();
}

template <> bool getValue<bool>(const json::value& val) {
    return val.as_bool();
}

template <typename T> T getFieldValue(const json::object& json_obj, const std::string& field_name) {
    auto it = json_obj.find(field_name);
    if (it != json_obj.end()) {
        return getValue<T>(it->value());
    } else {
        throw std::runtime_error("Field not found: " + field_name);
    }
}

template std::string getFieldValue<std::string>(const json::object&, const std::string&);
template uint64_t getFieldValue<uint64_t>(const json::object&, const std::string&);
template double getFieldValue<double>(const json::object&, const std::string&);
template bool getFieldValue<bool>(const json::object&, const std::string&);

std::string generateUuid() {
    static boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}
} // namespace utils
