// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include <numbers>
#include <optional>
#include <string>
#include <vector>

namespace odxf {

struct Coordinate2d final
{
    double x{ 0.0 };
    double y{ 0.0 };
};

struct Coordinate3d final
{
    double x{ 0.0 };
    double y{ 0.0 };
    double z{ 0.0 };
};

struct Vector3d final
{
    double x{ 0.0 };
    double y{ 0.0 };
    double z{ 0.0 };
};

struct Entity
{
    std::string layer;
    int color{ 0 };
};

struct Point final : Entity
{
    Coordinate3d coordinate;

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Ray final : Entity
{
    Coordinate3d startPoint;
    Vector3d direction;
};

struct Line final : Entity
{
    Coordinate3d start;
    Coordinate3d end;

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Circle final : Entity
{
    Coordinate3d center;
    double radius{ 0.0 };

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Arc final : Entity
{
    Coordinate3d center;
    double radius{ 0.0 };
    double startAngle{ 0.0 };
    double endAngle{ 0.0 };

    std::optional<Vector3d> extrusion;
    std::optional<double> thickness;
};

struct Ellipse final : Entity
{
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

struct LWPolyline final : Entity
{
    std::optional<double> elevation;
    bool isClosed{ false };
    Vertices vertices;
};

using Arcs = std::vector<Arc>;
using Circles = std::vector<Circle>;
using Ellipses = std::vector<Ellipse>;
using Lines = std::vector<Line>;
using Points = std::vector<Point>;
using LWPolylines = std::vector<LWPolyline>;
using Rays = std::vector<Ray>;

struct Entities final
{
    Arcs arcs;
    Circles circles;
    Ellipses ellipses;
    Lines lines;
    Points points;
    LWPolylines lwPolylines;
    Rays rays;
};

}   // namespace odxf
