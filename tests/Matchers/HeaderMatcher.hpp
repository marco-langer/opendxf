// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/header.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::Header> IsHeader(const odxf::Header& expected, double maxError = 1.0e-5);
