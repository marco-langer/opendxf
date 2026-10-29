// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/header.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::header> is_header(const odxf::header& expected, double max_error = 1.0e-5);
