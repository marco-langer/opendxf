// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/layer.hpp"

#include <string>
#include <vector>

namespace odxf {

struct LineType final
{
    std::string name;
    std::string displayName;
    int flags{ 64 };
};

using Layers = std::vector<Layer>;
using LineTypes = std::vector<LineType>;

struct Tables final
{
    LineTypes lineTypes;
    Layers layers;
};

}   // namespace odxf
