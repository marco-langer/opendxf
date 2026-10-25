// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

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
    Flags flags{ Flags::None };
    int lineType{ 0 };
};

}   // namespace odxf
