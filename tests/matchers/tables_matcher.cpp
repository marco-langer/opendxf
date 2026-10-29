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
            "complex_flags", &odxf::line_type_element::complex_flags, expected.complex_flags));
}

testing::Matcher<odxf::line_type> is_line_type(const odxf::line_type& expected)
{
    std::vector<testing::Matcher<odxf::line_type_element>> element_matchers;
    element_matchers.reserve(expected.elements.size());
    for (const odxf::line_type_element& element : expected.elements) {
        element_matchers.push_back(is_line_type_element(element));
    }

    return testing::AllOf(
        testing::Field("name", &odxf::line_type::name, expected.name),
        testing::Field("display_name", &odxf::line_type::display_name, expected.display_name),
        testing::Field("flags", &odxf::line_type::flags, expected.flags),
        testing::Field(
            "elements",
            &odxf::line_type::elements,
            testing::ElementsAreArray(std::move(element_matchers))));
}

testing::Matcher<odxf::line_types> are_line_types(const odxf::line_types& expected)
{
    std::vector<testing::Matcher<odxf::line_type>> element_matchers;
    element_matchers.reserve(expected.size());
    for (const odxf::line_type& line_type : expected) {
        element_matchers.push_back(is_line_type(line_type));
    }

    return testing::ElementsAreArray(std::move(element_matchers));
}

}   // namespace

testing::Matcher<odxf::tables> are_tables(const odxf::tables& expected)
{
    return testing::AllOf(
        testing::Field(
            "line_types", &odxf::tables::line_types, are_line_types(expected.line_types)),
        testing::Field("layers", &odxf::tables::layers, are_layers(expected.layers)));
}
