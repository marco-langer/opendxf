// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/layer.hpp"

#include <string>
#include <vector>

namespace odxf {

struct line_type_element final
{
    // In drawing units: a dash if positive, a gap if negative, a dot if 0.
    double length{ 0.0 };
    // Group code 74: 0 for a plain element. Otherwise the element embeds a text (2) or a shape (4),
    // whose details are skipped when reading and not written.
    int complexFlags{ 0 };
};

using line_type_elements = std::vector<line_type_element>;

struct line_type final
{
    std::string name;
    std::string displayName;
    int flags{ 64 };
    // Empty for a continuous line.
    line_type_elements elements{};
};

using layers = std::vector<layer>;
using line_types = std::vector<line_type>;

struct tables final
{
    line_types lineTypes;
    odxf::layers layers;
};

}   // namespace odxf
