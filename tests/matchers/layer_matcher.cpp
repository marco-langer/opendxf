// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "layer_matcher.hpp"

namespace {

testing::Matcher<odxf::layer> is_layer(const odxf::layer& expected)
{
    return testing::AllOf(
        testing::Field("name", &odxf::layer::name, expected.name),
        testing::Field("color", &odxf::layer::color, expected.color),
        testing::Field("trueColor", &odxf::layer::trueColor, expected.trueColor),
        testing::Field("lineweight", &odxf::layer::lineweight, expected.lineweight),
        testing::Field("lineType", &odxf::layer::lineType, expected.lineType),
        testing::Field("transparency", &odxf::layer::transparency, expected.transparency),
        testing::Field("flags", &odxf::layer::flags, expected.flags));
}

}   // namespace

testing::Matcher<std::vector<odxf::layer>> are_layers(const std::vector<odxf::layer>& expected)
{
    std::vector<testing::Matcher<odxf::layer>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::layer& expectedLayer : expected) {
        elementMatchers.push_back(is_layer(expectedLayer));
    }

    return testing::ElementsAreArray(elementMatchers);
}
