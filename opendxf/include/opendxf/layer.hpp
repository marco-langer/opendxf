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
    int color{ 7 };   // white
    Flags flags{ Flags::None };
    int lineType{ 0 };
};

}   // namespace odxf
