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

struct Vector3d final
{
    double x{ 0.0 };
    double y{ 0.0 };
    double z{ 0.0 };
};

// The properties all entities share.
struct EntityProperties final
{
    std::string layer;
    int color{ 256 };                 // BYLAYER
    std::optional<int> trueColor{};   // 0x00RRGGBB, takes precedence over color
    // hundredths of a millimeter, or -1 BYLAYER, -2 BYBLOCK, -3 DEFAULT
    int lineweight{ -1 };
    // The name of an entry in the LTYPE table, or BYLAYER or BYBLOCK.
    std::string lineType{ "BYLAYER" };
    double lineTypeScale{ 1.0 };
    EntityTransparency transparency{ ByLayer{} };
};

struct Point final
{
    EntityProperties properties;

    Coordinate3d coordinate;

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Ray final
{
    EntityProperties properties;

    Coordinate3d startPoint;
    Vector3d direction;
};

struct Line final
{
    EntityProperties properties;

    Coordinate3d start;
    Coordinate3d end;

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Circle final
{
    EntityProperties properties;

    Coordinate3d center;
    double radius{ 0.0 };

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Arc final
{
    EntityProperties properties;

    Coordinate3d center;
    double radius{ 0.0 };
    double startAngle{ 0.0 };
    double endAngle{ 0.0 };

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Ellipse final
{
    EntityProperties properties;

    Coordinate3d center;
    Coordinate3d endPointMajor;
    double axisRatio{ 1.0 };
    double startParameter{ 0.0 };
    double endParameter{ 2 * std::numbers::pi };

    std::optional<Vector3d> extrusion;
};

struct Vertex final
{
    Coordinate2d position;
    std::optional<double> bulge;
};

using Vertices = std::vector<Vertex>;

struct LWPolyline final
{
    EntityProperties properties;

    std::optional<double> elevation;
    bool isClosed{ false };
    Vertices vertices;
};

using Entity = std::variant<Arc, Circle, Ellipse, Line, LWPolyline, Point, Ray>;

// in file order, which is the default draw order
using Entities = std::vector<Entity>;

}   // namespace odxf
