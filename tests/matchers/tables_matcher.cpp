// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "tables_matcher.hpp"

#include "layer_matcher.hpp"

#include <vector>

namespace {

testing::Matcher<odxf::line_type_element>
is_line_type_element(const odxf::line_type_element& expected)
{
    return testing::AllOf(
        testing::Field(
            "length", &odxf::line_type_element::length, testing::DoubleNear(expected.length, 1e-9)),
        testing::Field(
            "complexFlags", &odxf::line_type_element::complexFlags, expected.complexFlags));
}

testing::Matcher<odxf::line_type> is_line_type(const odxf::line_type& expected)
{
    std::vector<testing::Matcher<odxf::line_type_element>> elementMatchers;
    elementMatchers.reserve(expected.elements.size());
    for (const odxf::line_type_element& element : expected.elements) {
        elementMatchers.push_back(is_line_type_element(element));
    }

    return testing::AllOf(
        testing::Field("name", &odxf::line_type::name, expected.name),
        testing::Field("displayName", &odxf::line_type::displayName, expected.displayName),
        testing::Field("flags", &odxf::line_type::flags, expected.flags),
        testing::Field(
            "elements",
            &odxf::line_type::elements,
            testing::ElementsAreArray(std::move(elementMatchers))));
}

testing::Matcher<odxf::line_types> are_line_types(const odxf::line_types& expected)
{
    std::vector<testing::Matcher<odxf::line_type>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::line_type& lineType : expected) {
        elementMatchers.push_back(is_line_type(lineType));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
}

}   // namespace

testing::Matcher<odxf::tables> are_tables(const odxf::tables& expected)
{
    return testing::AllOf(
        testing::Field("lineTypes", &odxf::tables::lineTypes, are_line_types(expected.lineTypes)),
        testing::Field("layers", &odxf::tables::layers, are_layers(expected.layers)));
}
