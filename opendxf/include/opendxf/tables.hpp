#pragma once

#include "opendxf/layer.hpp"

#include <string>
#include <vector>

namespace odxf {

struct LineType
{
    std::string name;
    std::string displayName;
    int flags{ 64 };
};

struct Tables
{
    std::vector<LineType> lineTypes;
    std::vector<Layer> layers;
};

}   // namespace odxf
