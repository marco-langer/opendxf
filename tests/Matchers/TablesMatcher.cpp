// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "TablesMatcher.hpp"

#include "LayerMatcher.hpp"

testing::Matcher<odxf::Tables> AreTables(const odxf::Tables& expected)
{
    return testing::AllOf(
        testing::Field("layers", &odxf::Tables::layers, AreLayers(expected.layers)));
}
