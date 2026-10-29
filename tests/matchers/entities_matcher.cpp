// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "entities_matcher.hpp"

#include "coordinate_matcher.hpp"
#include "overload.hpp"

#include <variant>

namespace {

testing::Matcher<odxf::Vector3d> IsVector(const odxf::Vector3d& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::Vector3d::x, testing::DoubleNear(expected.x, maxError)),
        testing::Field("y", &odxf::Vector3d::y, testing::DoubleNear(expected.y, maxError)),
        testing::Field("z", &odxf::Vector3d::z, testing::DoubleNear(expected.z, maxError)));
}

testing::Matcher<odxf::EntityProperties>
IsEntityProperties(const odxf::EntityProperties& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("layer", &odxf::EntityProperties::layer, expected.layer),
        testing::Field("color", &odxf::EntityProperties::color, expected.color),
        testing::Field("trueColor", &odxf::EntityProperties::trueColor, expected.trueColor),
        testing::Field("lineweight", &odxf::EntityProperties::lineweight, expected.lineweight),
        testing::Field("lineType", &odxf::EntityProperties::lineType, expected.lineType),
        testing::Field(
            "lineTypeScale",
            &odxf::EntityProperties::lineTypeScale,
            testing::DoubleNear(expected.lineTypeScale, maxError)),
        testing::Field(
            "transparency", &odxf::EntityProperties::transparency, expected.transparency));
}

testing::Matcher<odxf::Vertex> IsVertex(const odxf::Vertex& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "position", &odxf::Vertex::position, IsCoordinate(expected.position, maxError)),
        testing::Field("bulge", &odxf::Vertex::bulge, expected.bulge));
}

testing::Matcher<std::vector<odxf::Vertex>>
AreVertices(const odxf::Vertices& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::Vertex>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::Vertex& expectedVertex : expected) {
        elementMatchers.push_back(IsVertex(expectedVertex, maxError));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
}

testing::Matcher<const odxf::LWPolyline&>
IsLWPolyline(const odxf::LWPolyline& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::LWPolyline::properties,
            IsEntityProperties(expected.properties, maxError)),
        testing::Field("isClosed", &odxf::LWPolyline::isClosed, expected.isClosed),
        testing::Field(
            "vertices", &odxf::LWPolyline::vertices, AreVertices(expected.vertices, maxError)));
}

testing::Matcher<const odxf::Line&> IsLine(const odxf::Line& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::Line::properties,
            IsEntityProperties(expected.properties, maxError)),
        testing::Field("start", &odxf::Line::start, IsCoordinate(expected.start, maxError)),
        testing::Field("end", &odxf::Line::end, IsCoordinate(expected.end, maxError)));
}

testing::Matcher<const odxf::Circle&> IsCircle(const odxf::Circle& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::Circle::properties,
            IsEntityProperties(expected.properties, maxError)),
        testing::Field("center", &odxf::Circle::center, IsCoordinate(expected.center, maxError)),
        testing::Field(
            "radius", &odxf::Circle::radius, testing::DoubleNear(expected.radius, maxError)));
}

testing::Matcher<const odxf::Arc&> IsArc(const odxf::Arc& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::Arc::properties,
            IsEntityProperties(expected.properties, maxError)),
        testing::Field("center", &odxf::Arc::center, IsCoordinate(expected.center, maxError)),
        testing::Field(
            "radius", &odxf::Arc::radius, testing::DoubleNear(expected.radius, maxError)),
        testing::Field(
            "startAngle",
            &odxf::Arc::startAngle,
            testing::DoubleNear(expected.startAngle, maxError)),
        testing::Field(
            "endAngle", &odxf::Arc::endAngle, testing::DoubleNear(expected.endAngle, maxError)));
}

testing::Matcher<const odxf::Ellipse&> IsEllipse(const odxf::Ellipse& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::Ellipse::properties,
            IsEntityProperties(expected.properties, maxError)),
        testing::Field("center", &odxf::Ellipse::center, IsCoordinate(expected.center, maxError)),
        testing::Field(
            "endPointMajor",
            &odxf::Ellipse::endPointMajor,
            IsCoordinate(expected.endPointMajor, maxError)),
        testing::Field(
            "axisRatio",
            &odxf::Ellipse::axisRatio,
            testing::DoubleNear(expected.axisRatio, maxError)),
        testing::Field(
            "startParameter",
            &odxf::Ellipse::startParameter,
            testing::DoubleNear(expected.startParameter, maxError)),
        testing::Field(
            "endParameter",
            &odxf::Ellipse::endParameter,
            testing::DoubleNear(expected.endParameter, maxError)));
}

testing::Matcher<const odxf::Point&> IsPoint(const odxf::Point& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::Point::properties,
            IsEntityProperties(expected.properties, maxError)),
        testing::Field(
            "coordinate", &odxf::Point::coordinate, IsCoordinate(expected.coordinate, maxError)));
}

testing::Matcher<const odxf::Ray&> IsRay(const odxf::Ray& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::Ray::properties,
            IsEntityProperties(expected.properties, maxError)),
        testing::Field(
            "startPoint", &odxf::Ray::startPoint, IsCoordinate(expected.startPoint, maxError)),
        testing::Field("direction", &odxf::Ray::direction, IsVector(expected.direction, maxError)));
}

// Matches an entity of the same type as expected.
testing::Matcher<odxf::Entity> IsEntity(const odxf::Entity& expected, double maxError)
{
    return std::visit(
        odxf::overload{
            [maxError](const odxf::Arc& arc) -> testing::Matcher<odxf::Entity> {
                return testing::VariantWith<odxf::Arc>(IsArc(arc, maxError));
            },
            [maxError](const odxf::Circle& circle) -> testing::Matcher<odxf::Entity> {
                return testing::VariantWith<odxf::Circle>(IsCircle(circle, maxError));
            },
            [maxError](const odxf::Ellipse& ellipse) -> testing::Matcher<odxf::Entity> {
                return testing::VariantWith<odxf::Ellipse>(IsEllipse(ellipse, maxError));
            },
            [maxError](const odxf::Line& line) -> testing::Matcher<odxf::Entity> {
                return testing::VariantWith<odxf::Line>(IsLine(line, maxError));
            },
            [maxError](const odxf::LWPolyline& lwPolyline) -> testing::Matcher<odxf::Entity> {
                return testing::VariantWith<odxf::LWPolyline>(IsLWPolyline(lwPolyline, maxError));
            },
            [maxError](const odxf::Point& point) -> testing::Matcher<odxf::Entity> {
                return testing::VariantWith<odxf::Point>(IsPoint(point, maxError));
            },
            [maxError](const odxf::Ray& ray) -> testing::Matcher<odxf::Entity> {
                return testing::VariantWith<odxf::Ray>(IsRay(ray, maxError));
            },
        },
        expected);
}

}   // namespace

testing::Matcher<odxf::Entities> AreEntities(const odxf::Entities& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::Entity>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::Entity& expectedEntity : expected) {
        elementMatchers.push_back(IsEntity(expectedEntity, maxError));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
}
