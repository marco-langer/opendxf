// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"
#include "opendxf/properties.hpp"

#include <numbers>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace odxf {

struct vector3d final
{
    double x{ 0.0 };
    double y{ 0.0 };
    double z{ 0.0 };
};

// The properties all entities share.
struct entity_properties final
{
    std::string layer;
    int color{ 256 };                  // BYLAYER
    std::optional<int> true_color{};   // 0x00RRGGBB, takes precedence over color
    // hundredths of a millimeter, or -1 BYLAYER, -2 BYBLOCK, -3 DEFAULT
    int lineweight{ -1 };
    // The name of an entry in the LTYPE table, or BYLAYER or BYBLOCK.
    std::string line_type{ "BYLAYER" };
    double line_type_scale{ 1.0 };
    entity_transparency transparency{ by_layer{} };
    bool is_visible{ true };
};

struct point final
{
    entity_properties properties;

    coordinate3d coordinate;

    std::optional<vector3d> extrusion;
    std::optional<double> thickness;
};

struct ray final
{
    entity_properties properties;

    coordinate3d start_point;
    vector3d direction;
};

struct line final
{
    entity_properties properties;

    coordinate3d start;
    coordinate3d end;

    std::optional<vector3d> extrusion;
    std::optional<double> thickness;
};

struct circle final
{
    entity_properties properties;

    coordinate3d center;
    double radius{ 0.0 };

    std::optional<vector3d> extrusion;
    std::optional<double> thickness;
};

struct arc final
{
    entity_properties properties;

    coordinate3d center;
    double radius{ 0.0 };
    double start_angle{ 0.0 };
    double end_angle{ 0.0 };

    std::optional<vector3d> extrusion;
    std::optional<double> thickness;
};

struct ellipse final
{
    entity_properties properties;

    coordinate3d center;
    coordinate3d end_point_major;
    double axis_ratio{ 1.0 };
    double start_parameter{ 0.0 };
    double end_parameter{ 2 * std::numbers::pi };

    std::optional<vector3d> extrusion;
};

struct vertex final
{
    coordinate2d position;
    std::optional<double> bulge;
};

using vertices = std::vector<vertex>;

struct lw_polyline final
{
    entity_properties properties;

    std::optional<double> elevation;
    bool is_closed{ false };
    odxf::vertices vertices;
};

using entity = std::variant<arc, circle, ellipse, line, lw_polyline, point, ray>;

// in file order, which is the default draw order
using entities = std::vector<entity>;

}   // namespace odxf
