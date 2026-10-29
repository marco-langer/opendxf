// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "layer_matcher.hpp"

namespace {

testing::Matcher<odxf::layer> is_layer(const odxf::layer& expected)
{
    return testing::AllOf(
        testing::Field("name", &odxf::layer::name, expected.name),
        testing::Field("color", &odxf::layer::color, expected.color),
        testing::Field("true_color", &odxf::layer::true_color, expected.true_color),
        testing::Field("lineweight", &odxf::layer::lineweight, expected.lineweight),
        testing::Field("line_type", &odxf::layer::line_type, expected.line_type),
        testing::Field("transparency", &odxf::layer::transparency, expected.transparency),
        testing::Field("flags", &odxf::layer::flags, expected.flags));
}

}   // namespace

testing::Matcher<std::vector<odxf::layer>> are_layers(const std::vector<odxf::layer>& expected)
{
    std::vector<testing::Matcher<odxf::layer>> element_matchers;
    element_matchers.reserve(expected.size());
    for (const odxf::layer& expected_layer : expected) {
        element_matchers.push_back(is_layer(expected_layer));
    }

    return testing::ElementsAreArray(element_matchers);
}
