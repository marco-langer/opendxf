// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "CoordinateMatcher.hpp"

testing::Matcher<const odxf::Coordinate2d&>
IsCoordinate(const odxf::Coordinate2d& coordinate, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::Coordinate2d::x, testing::DoubleNear(coordinate.x, maxError)),
        testing::Field("y", &odxf::Coordinate2d::y, testing::DoubleNear(coordinate.y, maxError)));
}

testing::Matcher<const odxf::Coordinate3d&>
IsCoordinate(const odxf::Coordinate3d& coordinate, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::Coordinate3d::x, testing::DoubleNear(coordinate.x, maxError)),
        testing::Field("y", &odxf::Coordinate3d::y, testing::DoubleNear(coordinate.y, maxError)),
        testing::Field("z", &odxf::Coordinate3d::z, testing::DoubleNear(coordinate.z, maxError)));
}
