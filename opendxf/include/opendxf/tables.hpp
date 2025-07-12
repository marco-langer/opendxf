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

using Layers = std::vector<Layer>;
using LineTypes = std::vector<LineType>;

struct Tables
{
    LineTypes lineTypes;
    Layers layers;
};

}   // namespace odxf
