#include "EntitiesMatcher.hpp"

namespace {

testing::Matcher<odxf::Coordinate2d>
IsCoordinate(const odxf::Coordinate2d& coordinate, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::Coordinate2d::x, testing::DoubleNear(coordinate.x, maxError)),
        testing::Field("y", &odxf::Coordinate2d::y, testing::DoubleNear(coordinate.y, maxError)));
}

testing::Matcher<odxf::Coordinate3d>
IsCoordinate(const odxf::Coordinate3d& coordinate, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::Coordinate3d::x, testing::DoubleNear(coordinate.x, maxError)),
        testing::Field("y", &odxf::Coordinate3d::y, testing::DoubleNear(coordinate.y, maxError)),
        testing::Field("z", &odxf::Coordinate3d::z, testing::DoubleNear(coordinate.z, maxError)));
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

testing::Matcher<odxf::LWPolyline> IsLWPolyline(const odxf::LWPolyline& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("isClosed", &odxf::LWPolyline::isClosed, expected.isClosed),
        testing::Field(
            "vertices", &odxf::LWPolyline::vertices, AreVertices(expected.vertices, maxError)));
}

testing::Matcher<odxf::LWPolylines>
AreLWPolylines(const odxf::LWPolylines& expected, double maxError)
{
    std::vector<testing::Matcher<odxf::LWPolyline>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::LWPolyline& expectedLWPolyline : expected) {
        elementMatchers.push_back(IsLWPolyline(expectedLWPolyline, maxError));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
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
            "radius", &odxf::Circle::radius, testing::DoubleNear(expected.radius, maxError)));
}

testing::Matcher<odxf::Arc> IsArc(const odxf::Arc& expected, double maxError)
{
    return testing::AllOf(
        testing::Field("layer", &odxf::Entity::layer, expected.layer),
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
        testing::Field("arcs", &odxf::Entities::arcs, AreArcs(expected.arcs, maxError)),
        testing::Field(
            "lwPolylines",
            &odxf::Entities::lwPolylines,
            AreLWPolylines(expected.lwPolylines, maxError)));
}
