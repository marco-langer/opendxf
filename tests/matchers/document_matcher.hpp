// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/document.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::document> is_document(const odxf::document& expected);
