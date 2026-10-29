// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/properties.hpp"

#include <optional>
#include <string>

namespace odxf {

struct layer final
{
    enum flag
    {
        none = 0,
        frozen = 1,
        default_frozen = 2,
        locked = 4
    };

    std::string name;
    // 7 is white or black, whichever contrasts with the background. Negative if the layer is off.
    int color{ 7 };
    std::optional<int> true_color{};   // 0x00RRGGBB, takes precedence over color
    flag flags{ flag::none };
    // hundredths of a millimeter, or -3 DEFAULT
    int lineweight{ -3 };
    std::string line_type{ "CONTINUOUS" };
    // Empty means opaque.
    std::optional<odxf::transparency> transparency{};
};

}   // namespace odxf
