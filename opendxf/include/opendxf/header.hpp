#pragma once

#include <string>
#include <unordered_map>
#include <variant>

namespace odxf {

struct Header
{
    std::unordered_map<std::string, std::variant<std::string>> entries;
};

}   // namespace odxf
