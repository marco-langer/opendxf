// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/tables.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::tables> are_tables(const odxf::tables& expected);
