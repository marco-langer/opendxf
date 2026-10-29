// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "coordinate_matcher.hpp"

testing::Matcher<const odxf::coordinate2d&>
is_coordinate(const odxf::coordinate2d& coordinate, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::coordinate2d::x, testing::DoubleNear(coordinate.x, maxError)),
        testing::Field("y", &odxf::coordinate2d::y, testing::DoubleNear(coordinate.y, maxError)));
}

testing::Matcher<const odxf::coordinate3d&>
is_coordinate(const odxf::coordinate3d& coordinate, double maxError)
{
    return testing::AllOf(
        testing::Field("x", &odxf::coordinate3d::x, testing::DoubleNear(coordinate.x, maxError)),
        testing::Field("y", &odxf::coordinate3d::y, testing::DoubleNear(coordinate.y, maxError)),
        testing::Field("z", &odxf::coordinate3d::z, testing::DoubleNear(coordinate.z, maxError)));
}
