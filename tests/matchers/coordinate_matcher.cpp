// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "coordinate_matcher.hpp"

testing::Matcher<const odxf::coordinate2d&>
is_coordinate(const odxf::coordinate2d& coordinate, double max_error)
{
    return testing::AllOf(
        testing::Field("x", &odxf::coordinate2d::x, testing::DoubleNear(coordinate.x, max_error)),
        testing::Field("y", &odxf::coordinate2d::y, testing::DoubleNear(coordinate.y, max_error)));
}

testing::Matcher<const odxf::coordinate3d&>
is_coordinate(const odxf::coordinate3d& coordinate, double max_error)
{
    return testing::AllOf(
        testing::Field("x", &odxf::coordinate3d::x, testing::DoubleNear(coordinate.x, max_error)),
        testing::Field("y", &odxf::coordinate3d::y, testing::DoubleNear(coordinate.y, max_error)),
        testing::Field("z", &odxf::coordinate3d::z, testing::DoubleNear(coordinate.z, max_error)));
}
