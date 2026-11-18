// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "drawing_matcher.hpp"

#include "entities_matcher.hpp"
#include "header_matcher.hpp"
#include "tables_matcher.hpp"

testing::Matcher<odxf::drawing> is_drawing(const odxf::drawing& expected)
{
    return testing::AllOf(
        testing::Field("header", &odxf::drawing::header, is_header(expected.header)),
        testing::Field("tables", &odxf::drawing::tables, are_tables(expected.tables)),
        testing::Field("entities", &odxf::drawing::entities, are_entities(expected.entities)));
}
