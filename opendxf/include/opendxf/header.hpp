// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include <string>
#include <unordered_map>
#include <variant>

namespace odxf {

using HeaderKey = std::string;
using HeaderValue = std::variant<std::string>;
using HeaderEntry = std::pair<HeaderKey, HeaderValue>;

struct Header final
{
    std::unordered_map<HeaderKey, HeaderValue> entries;
};

}   // namespace odxf
