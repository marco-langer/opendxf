// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/entities.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::entities>
are_entities(const odxf::entities& expected, double max_error = 1.0e-5);
