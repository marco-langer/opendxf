// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace odxf {

using header_key = std::string;
using header_value = std::variant<bool, int, double, std::string, coordinate2d, coordinate3d>;
using header_entry = std::pair<header_key, header_value>;

// transparent hash, enables lookups by std::string_view without constructing a header_key
struct header_key_hash final
{
    using is_transparent = void;

    std::size_t operator()(std::string_view key) const noexcept
    {
        return std::hash<std::string_view>{}(key);
    }
};

using header_entries =
    std::unordered_map<header_key, header_value, header_key_hash, std::equal_to<>>;

struct header final
{
    header_entries entries;
};

}   // namespace odxf
