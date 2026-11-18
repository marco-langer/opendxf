// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/drawing.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::drawing> is_drawing(const odxf::drawing& expected);
