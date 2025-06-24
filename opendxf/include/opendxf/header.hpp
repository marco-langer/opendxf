#pragma once

#include <string>
#include <unordered_map>
#include <variant>

namespace odxf {

using HeaderKey = std::string;
using HeaderValue = std::variant<std::string>;

struct Header
{
    std::unordered_map<HeaderKey, HeaderValue> entries;
};

}   // namespace odxf
