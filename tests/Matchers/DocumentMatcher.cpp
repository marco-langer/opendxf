// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "DocumentMatcher.hpp"

#include "EntitiesMatcher.hpp"
#include "HeaderMatcher.hpp"
#include "TablesMatcher.hpp"

testing::Matcher<odxf::Document> IsDocument(const odxf::Document& expected)
{
    return testing::AllOf(
        testing::Field("header", &odxf::Document::header, IsHeader(expected.header)),
        testing::Field("tables", &odxf::Document::tables, AreTables(expected.tables)),
        testing::Field("entities", &odxf::Document::entities, AreEntities(expected.entities)));
}
