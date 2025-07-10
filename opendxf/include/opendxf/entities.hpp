#pragma once

#include <numbers>
#include <optional>
#include <string>
#include <vector>

namespace odxf {

struct Coordinate3d
{
    float x{ 0.0F };
    float y{ 0.0F };
    float z{ 0.0F };
};

struct Vector3d
{
    float x{ 0.0F };
    float y{ 0.0F };
    float z{ 0.0F };
};

struct Entity
{
    std::string layer;
    int color{ 0 };
};

struct Point : Entity
{
    Coordinate3d coordinate;

    std::optional<Vector3d> extrusion;
    std::optional<float> thickness;
};

struct Ray : Entity
{
    Coordinate3d startPoint;
    Vector3d direction;
};

struct Line : Entity
{
    Coordinate3d start;
    Coordinate3d end;

    std::optional<Vector3d> extrusion;
    std::optional<float> thickness;
};

struct Circle : Entity
{
    Coordinate3d center;
    float radius{ 0.0F };

    std::optional<Vector3d> extrusion;
    std::optional<float> thickness;
};

struct Arc : Entity
{
    Coordinate3d center;
    float radius{ 0.0F };
    float startAngle{ 0.0F };
    float endAngle{ 0.0F };

    std::optional<Vector3d> extrusion;
    std::optional<float> thickness;
};

struct Ellipse : Entity
{
    Coordinate3d center;
    Coordinate3d endPointMajor;
    float axisRatio{ 1.0F };
    float startParameter{ 0.0F };
    float endParameter{ 2 * std::numbers::pi_v<float> };

    std::optional<Vector3d> extrusion;
};

using Lines = std::vector<Line>;
using Circles = std::vector<Circle>;
using Arcs = std::vector<Arc>;

struct Entities
{
    std::vector<Point> points;
    std::vector<Ray> rays;
    Lines lines;
    Circles circles;
    Arcs arcs;
    std::vector<Ellipse> ellipses;
};

}   // namespace odxf
