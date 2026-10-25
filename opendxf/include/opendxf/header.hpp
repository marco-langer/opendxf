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

using HeaderKey = std::string;
using HeaderValue = std::variant<bool, int, double, std::string, Coordinate2d, Coordinate3d>;
using HeaderEntry = std::pair<HeaderKey, HeaderValue>;

// transparent hash, enables lookups by std::string_view without constructing a HeaderKey
struct HeaderKeyHash final
{
    using is_transparent = void;

    std::size_t operator()(std::string_view key) const noexcept
    {
        return std::hash<std::string_view>{}(key);
    }
};

using HeaderEntries = std::unordered_map<HeaderKey, HeaderValue, HeaderKeyHash, std::equal_to<>>;

struct Header final
{
    HeaderEntries entries;
};

}   // namespace odxf
