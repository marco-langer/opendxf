// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"

#include <gmock/gmock.h>

testing::Matcher<const odxf::coordinate2d&>
is_coordinate(const odxf::coordinate2d& coordinate, double maxError = 1.0e-5);

testing::Matcher<const odxf::coordinate3d&>
is_coordinate(const odxf::coordinate3d& coordinate, double maxError = 1.0e-5);
