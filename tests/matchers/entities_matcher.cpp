// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "entities_matcher.hpp"

#include "coordinate_matcher.hpp"
#include "overload.hpp"

#include <variant>

namespace {

testing::Matcher<odxf::vector3d> is_vector(const odxf::vector3d& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::vector3d::x, testing::DoubleNear(expected.x, maxError)),
        testing::Field("y", &odxf::vector3d::y, testing::DoubleNear(expected.y, maxError)),
        testing::Field("z", &odxf::vector3d::z, testing::DoubleNear(expected.z, maxError)));
}

testing::Matcher<odxf::entity_properties>
is_entity_properties(const odxf::entity_properties& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("layer", &odxf::entity_properties::layer, expected.layer),
        testing::Field("color", &odxf::entity_properties::color, expected.color),
        testing::Field("trueColor", &odxf::entity_properties::trueColor, expected.trueColor),
        testing::Field("lineweight", &odxf::entity_properties::lineweight, expected.lineweight),
        testing::Field("lineType", &odxf::entity_properties::lineType, expected.lineType),
        testing::Field(
            "lineTypeScale",
            &odxf::entity_properties::lineTypeScale,
            testing::DoubleNear(expected.lineTypeScale, maxError)),
        testing::Field(
            "transparency", &odxf::entity_properties::transparency, expected.transparency));
}

testing::Matcher<odxf::vertex> is_vertex(const odxf::vertex& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "position", &odxf::vertex::position, is_coordinate(expected.position, maxError)),
        testing::Field("bulge", &odxf::vertex::bulge, expected.bulge));
}

testing::Matcher<std::vector<odxf::vertex>>
are_vertices(const odxf::vertices& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::vertex>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::vertex& expectedVertex : expected) {
        elementMatchers.push_back(is_vertex(expectedVertex, maxError));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
}

testing::Matcher<const odxf::lw_polyline&>
is_lw_polyline(const odxf::lw_polyline& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::lw_polyline::properties,
            is_entity_properties(expected.properties, maxError)),
        testing::Field("isClosed", &odxf::lw_polyline::isClosed, expected.isClosed),
        testing::Field(
            "vertices", &odxf::lw_polyline::vertices, are_vertices(expected.vertices, maxError)));
}

testing::Matcher<const odxf::line&> is_line(const odxf::line& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::line::properties,
            is_entity_properties(expected.properties, maxError)),
        testing::Field("start", &odxf::line::start, is_coordinate(expected.start, maxError)),
        testing::Field("end", &odxf::line::end, is_coordinate(expected.end, maxError)));
}

testing::Matcher<const odxf::circle&> is_circle(const odxf::circle& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::circle::properties,
            is_entity_properties(expected.properties, maxError)),
        testing::Field("center", &odxf::circle::center, is_coordinate(expected.center, maxError)),
        testing::Field(
            "radius", &odxf::circle::radius, testing::DoubleNear(expected.radius, maxError)));
}

testing::Matcher<const odxf::arc&> is_arc(const odxf::arc& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::arc::properties,
            is_entity_properties(expected.properties, maxError)),
        testing::Field("center", &odxf::arc::center, is_coordinate(expected.center, maxError)),
        testing::Field(
            "radius", &odxf::arc::radius, testing::DoubleNear(expected.radius, maxError)),
        testing::Field(
            "startAngle",
            &odxf::arc::startAngle,
            testing::DoubleNear(expected.startAngle, maxError)),
        testing::Field(
            "endAngle", &odxf::arc::endAngle, testing::DoubleNear(expected.endAngle, maxError)));
}

testing::Matcher<const odxf::ellipse&> is_ellipse(const odxf::ellipse& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::ellipse::properties,
            is_entity_properties(expected.properties, maxError)),
        testing::Field("center", &odxf::ellipse::center, is_coordinate(expected.center, maxError)),
        testing::Field(
            "endPointMajor",
            &odxf::ellipse::endPointMajor,
            is_coordinate(expected.endPointMajor, maxError)),
        testing::Field(
            "axisRatio",
            &odxf::ellipse::axisRatio,
            testing::DoubleNear(expected.axisRatio, maxError)),
        testing::Field(
            "startParameter",
            &odxf::ellipse::startParameter,
            testing::DoubleNear(expected.startParameter, maxError)),
        testing::Field(
            "endParameter",
            &odxf::ellipse::endParameter,
            testing::DoubleNear(expected.endParameter, maxError)));
}

testing::Matcher<const odxf::point&> is_point(const odxf::point& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::point::properties,
            is_entity_properties(expected.properties, maxError)),
        testing::Field(
            "coordinate", &odxf::point::coordinate, is_coordinate(expected.coordinate, maxError)));
}

testing::Matcher<const odxf::ray&> is_ray(const odxf::ray& expected, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::ray::properties,
            is_entity_properties(expected.properties, maxError)),
        testing::Field(
            "startPoint", &odxf::ray::startPoint, is_coordinate(expected.startPoint, maxError)),
        testing::Field(
            "direction", &odxf::ray::direction, is_vector(expected.direction, maxError)));
}

// Matches an entity of the same type as expected.
testing::Matcher<odxf::entity> is_entity(const odxf::entity& expected, double maxError)
{
    return std::visit(
        odxf::overload{
            [maxError](const odxf::arc& arc) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::arc>(is_arc(arc, maxError));
            },
            [maxError](const odxf::circle& circle) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::circle>(is_circle(circle, maxError));
            },
            [maxError](const odxf::ellipse& ellipse) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::ellipse>(is_ellipse(ellipse, maxError));
            },
            [maxError](const odxf::line& line) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::line>(is_line(line, maxError));
            },
            [maxError](const odxf::lw_polyline& lwPolyline) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::lw_polyline>(
                    is_lw_polyline(lwPolyline, maxError));
            },
            [maxError](const odxf::point& point) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::point>(is_point(point, maxError));
            },
            [maxError](const odxf::ray& ray) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::ray>(is_ray(ray, maxError));
            },
        },
        expected);
}

}   // namespace

testing::Matcher<odxf::entities> are_entities(const odxf::entities& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::entity>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::entity& expectedEntity : expected) {
        elementMatchers.push_back(is_entity(expectedEntity, maxError));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
}
