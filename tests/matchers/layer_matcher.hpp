// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/layer.hpp"

#include <gmock/gmock.h>

testing::Matcher<std::vector<odxf::layer>> are_layers(const std::vector<odxf::layer>& expected);
