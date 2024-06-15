#pragma once

#include <string>
#include <vector>

namespace odxf {

struct LineType
{
    std::string name;
    std::string displayName;
    int flags{ 64 };
};

struct Layer
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

struct Tables
{
    std::vector<LineType> lineTypes;
    std::vector<Layer> layers;
};

}   // namespace odxf
