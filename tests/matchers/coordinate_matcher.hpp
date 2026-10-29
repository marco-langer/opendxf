// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"

#include <gmock/gmock.h>

testing::Matcher<const odxf::Coordinate2d&>
IsCoordinate(const odxf::Coordinate2d& coordinate, double maxError = 1.0e-5);

testing::Matcher<const odxf::Coordinate3d&>
IsCoordinate(const odxf::Coordinate3d& coordinate, double maxError = 1.0e-5);
