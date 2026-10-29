// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "document_matcher.hpp"

#include "entities_matcher.hpp"
#include "header_matcher.hpp"
#include "tables_matcher.hpp"

testing::Matcher<odxf::document> is_document(const odxf::document& expected)
{
    return testing::AllOf(
        testing::Field("header", &odxf::document::header, is_header(expected.header)),
        testing::Field("tables", &odxf::document::tables, are_tables(expected.tables)),
        testing::Field("entities", &odxf::document::entities, are_entities(expected.entities)));
}
