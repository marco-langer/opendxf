// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/properties.hpp"

#include <optional>
#include <string>

namespace odxf {

struct Layer final
{
    enum Flags
    {
        None = 0,
        Frozen = 1,
        DefaultFrozen = 2,
        Locked = 4
    };

    std::string name;
    // 7 is white or black, whichever contrasts with the background. Negative if the layer is off.
    int color{ 7 };
    std::optional<int> trueColor{};   // 0x00RRGGBB, takes precedence over color
    Flags flags{ Flags::None };
    // hundredths of a millimeter, or -3 DEFAULT
    int lineweight{ -3 };
    std::string lineType{ "CONTINUOUS" };
    // Empty means opaque.
    std::optional<Transparency> transparency{};
};

}   // namespace odxf
