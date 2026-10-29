// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "entities_matcher.hpp"

#include "coordinate_matcher.hpp"
#include "overload.hpp"

#include <variant>

namespace {

testing::Matcher<odxf::vector3d> is_vector(const odxf::vector3d& expected, double max_error)
{
    return testing::AllOf(
        testing::Field("x", &odxf::vector3d::x, testing::DoubleNear(expected.x, max_error)),
        testing::Field("y", &odxf::vector3d::y, testing::DoubleNear(expected.y, max_error)),
        testing::Field("z", &odxf::vector3d::z, testing::DoubleNear(expected.z, max_error)));
}

testing::Matcher<odxf::entity_properties>
is_entity_properties(const odxf::entity_properties& expected, double max_error)
{
    return testing::AllOf(
        testing::Field("layer", &odxf::entity_properties::layer, expected.layer),
        testing::Field("color", &odxf::entity_properties::color, expected.color),
        testing::Field("true_color", &odxf::entity_properties::true_color, expected.true_color),
        testing::Field("lineweight", &odxf::entity_properties::lineweight, expected.lineweight),
        testing::Field("line_type", &odxf::entity_properties::line_type, expected.line_type),
        testing::Field(
            "line_type_scale",
            &odxf::entity_properties::line_type_scale,
            testing::DoubleNear(expected.line_type_scale, max_error)),
        testing::Field(
            "transparency", &odxf::entity_properties::transparency, expected.transparency));
}

testing::Matcher<odxf::vertex> is_vertex(const odxf::vertex& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "position", &odxf::vertex::position, is_coordinate(expected.position, max_error)),
        testing::Field("bulge", &odxf::vertex::bulge, expected.bulge));
}

testing::Matcher<std::vector<odxf::vertex>>
are_vertices(const odxf::vertices& expected, double max_error)
{
    std::vector<testing::Matcher<odxf::vertex>> element_matchers;
    element_matchers.reserve(expected.size());
    for (const odxf::vertex& expected_vertex : expected) {
        element_matchers.push_back(is_vertex(expected_vertex, max_error));
    }

    return testing::ElementsAreArray(std::move(element_matchers));
}

testing::Matcher<const odxf::lw_polyline&>
is_lw_polyline(const odxf::lw_polyline& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::lw_polyline::properties,
            is_entity_properties(expected.properties, max_error)),
        testing::Field("is_closed", &odxf::lw_polyline::is_closed, expected.is_closed),
        testing::Field(
            "vertices", &odxf::lw_polyline::vertices, are_vertices(expected.vertices, max_error)));
}

testing::Matcher<const odxf::line&> is_line(const odxf::line& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::line::properties,
            is_entity_properties(expected.properties, max_error)),
        testing::Field("start", &odxf::line::start, is_coordinate(expected.start, max_error)),
        testing::Field("end", &odxf::line::end, is_coordinate(expected.end, max_error)));
}

testing::Matcher<const odxf::circle&> is_circle(const odxf::circle& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::circle::properties,
            is_entity_properties(expected.properties, max_error)),
        testing::Field("center", &odxf::circle::center, is_coordinate(expected.center, max_error)),
        testing::Field(
            "radius", &odxf::circle::radius, testing::DoubleNear(expected.radius, max_error)));
}

testing::Matcher<const odxf::arc&> is_arc(const odxf::arc& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::arc::properties,
            is_entity_properties(expected.properties, max_error)),
        testing::Field("center", &odxf::arc::center, is_coordinate(expected.center, max_error)),
        testing::Field(
            "radius", &odxf::arc::radius, testing::DoubleNear(expected.radius, max_error)),
        testing::Field(
            "start_angle",
            &odxf::arc::start_angle,
            testing::DoubleNear(expected.start_angle, max_error)),
        testing::Field(
            "end_angle",
            &odxf::arc::end_angle,
            testing::DoubleNear(expected.end_angle, max_error)));
}

testing::Matcher<const odxf::ellipse&> is_ellipse(const odxf::ellipse& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::ellipse::properties,
            is_entity_properties(expected.properties, max_error)),
        testing::Field("center", &odxf::ellipse::center, is_coordinate(expected.center, max_error)),
        testing::Field(
            "end_point_major",
            &odxf::ellipse::end_point_major,
            is_coordinate(expected.end_point_major, max_error)),
        testing::Field(
            "axis_ratio",
            &odxf::ellipse::axis_ratio,
            testing::DoubleNear(expected.axis_ratio, max_error)),
        testing::Field(
            "start_parameter",
            &odxf::ellipse::start_parameter,
            testing::DoubleNear(expected.start_parameter, max_error)),
        testing::Field(
            "end_parameter",
            &odxf::ellipse::end_parameter,
            testing::DoubleNear(expected.end_parameter, max_error)));
}

testing::Matcher<const odxf::point&> is_point(const odxf::point& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::point::properties,
            is_entity_properties(expected.properties, max_error)),
        testing::Field(
            "coordinate", &odxf::point::coordinate, is_coordinate(expected.coordinate, max_error)));
}

testing::Matcher<const odxf::ray&> is_ray(const odxf::ray& expected, double max_error)
{
    return testing::AllOf(
        testing::Field(
            "properties",
            &odxf::ray::properties,
            is_entity_properties(expected.properties, max_error)),
        testing::Field(
            "start_point", &odxf::ray::start_point, is_coordinate(expected.start_point, max_error)),
        testing::Field(
            "direction", &odxf::ray::direction, is_vector(expected.direction, max_error)));
}

// Matches an entity of the same type as expected.
testing::Matcher<odxf::entity> is_entity(const odxf::entity& expected, double max_error)
{
    return std::visit(
        odxf::overload{
            [max_error](const odxf::arc& arc) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::arc>(is_arc(arc, max_error));
            },
            [max_error](const odxf::circle& circle) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::circle>(is_circle(circle, max_error));
            },
            [max_error](const odxf::ellipse& ellipse) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::ellipse>(is_ellipse(ellipse, max_error));
            },
            [max_error](const odxf::line& line) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::line>(is_line(line, max_error));
            },
            [max_error](const odxf::lw_polyline& lw_polyline) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::lw_polyline>(
                    is_lw_polyline(lw_polyline, max_error));
            },
            [max_error](const odxf::point& point) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::point>(is_point(point, max_error));
            },
            [max_error](const odxf::ray& ray) -> testing::Matcher<odxf::entity> {
                return testing::VariantWith<odxf::ray>(is_ray(ray, max_error));
            },
        },
        expected);
}

}   // namespace

testing::Matcher<odxf::entities> are_entities(const odxf::entities& expected, double max_error)
{
    std::vector<testing::Matcher<odxf::entity>> element_matchers;
    element_matchers.reserve(expected.size());
    for (const odxf::entity& expected_entity : expected) {
        element_matchers.push_back(is_entity(expected_entity, max_error));
    }

    return testing::ElementsAreArray(std::move(element_matchers));
}
