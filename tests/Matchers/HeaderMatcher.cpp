// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "HeaderMatcher.hpp"

testing::Matcher<odxf::Header> IsHeader(const odxf::Header& expected)
{
    return testing::Field("entries", &odxf::Header::entries, expected.entries);
}
