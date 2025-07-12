#include "EntitiesMatcher.hpp"

namespace {

testing::Matcher<odxf::Coordinate3d>
IsCoordinate(const odxf::Coordinate3d& coordinate, double maxError)
{
    return testing::AllOf(
        testing::Field(
            "x",
            &odxf::Coordinate3d::x,
            testing::FloatNear(coordinate.x, static_cast<float>(maxError))),
        testing::Field(
            "y",
            &odxf::Coordinate3d::y,
            testing::FloatNear(coordinate.y, static_cast<float>(maxError))),
        testing::Field(
            "z",
            &odxf::Coordinate3d::z,
            testing::FloatNear(coordinate.z, static_cast<float>(maxError))));
}

testing::Matcher<odxf::Line> IsLine(const odxf::Line& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("layer", &odxf::Entity::layer, expected.layer),
        testing::Field("start", &odxf::Line::start, IsCoordinate(expected.start, maxError)),
        testing::Field("end", &odxf::Line::end, IsCoordinate(expected.end, maxError)));
}

testing::Matcher<odxf::Circle> IsCircle(const odxf::Circle& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("layer", &odxf::Entity::layer, expected.layer),
        testing::Field("center", &odxf::Circle::center, IsCoordinate(expected.center, maxError)),
        testing::Field(
            "radius",
            &odxf::Circle::radius,
            testing::FloatNear(expected.radius, static_cast<float>(maxError))));
}

testing::Matcher<odxf::Arc> IsArc(const odxf::Arc& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("layer", &odxf::Entity::layer, expected.layer),
        testing::Field("center", &odxf::Arc::center, IsCoordinate(expected.center, maxError)),
        testing::Field(
            "radius",
            &odxf::Arc::radius,
            testing::FloatNear(expected.radius, static_cast<float>(maxError))),
        testing::Field(
            "startAngle",
            &odxf::Arc::startAngle,
            testing::FloatNear(expected.startAngle, static_cast<float>(maxError))),
        testing::Field(
            "endAngle",
            &odxf::Arc::endAngle,
            testing::FloatNear(expected.endAngle, static_cast<float>(maxError))));
}

testing::Matcher<std::vector<odxf::Line>>
AreLines(const std::vector<odxf::Line>& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::Line>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::Line& expectedLine : expected) {
        elementMatchers.push_back(IsLine(expectedLine, maxError));
    }

    return testing::ElementsAreArray(elementMatchers);
}

testing::Matcher<std::vector<odxf::Circle>>
AreCircles(const std::vector<odxf::Circle>& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::Circle>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::Circle& expectedCircle : expected) {
        elementMatchers.push_back(IsCircle(expectedCircle, maxError));
    }

    return testing::ElementsAreArray(elementMatchers);
}

testing::Matcher<odxf::Arcs> AreArcs(const odxf::Arcs& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::Arc>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::Arc& expectedArc : expected) {
        elementMatchers.push_back(IsArc(expectedArc, maxError));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
}

}   // namespace

testing::Matcher<odxf::Entities> AreEntities(const odxf::Entities& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("lines", &odxf::Entities::lines, AreLines(expected.lines, maxError)),
        testing::Field("circles", &odxf::Entities::circles, AreCircles(expected.circles, maxError)),
        testing::Field("arcs", &odxf::Entities::arcs, AreArcs(expected.arcs, maxError)));
}
