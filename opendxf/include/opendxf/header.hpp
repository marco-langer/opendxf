// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"

#include <string>
#include <unordered_map>
#include <variant>

namespace odxf {

using HeaderKey = std::string;
using HeaderValue = std::variant<int, double, std::string, Coordinate2d, Coordinate3d>;
using HeaderEntry = std::pair<HeaderKey, HeaderValue>;

struct Header final
{
    std::unordered_map<HeaderKey, HeaderValue> entries;
};

}   // namespace odxf
