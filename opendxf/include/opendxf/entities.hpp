#pragma once

#include <numbers>
#include <optional>
#include <string>
#include <vector>

namespace odxf {

struct Coordinate2d
{
    float x{ 0.0F };
    float y{ 0.0F };
};

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

struct Vertex
{
    Coordinate2d position;
    std::optional<float> bulge;
};

using Vertices = std::vector<Vertex>;

struct LWPolyline : Entity
{
    std::optional<float> elevation;
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

struct Entities
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
