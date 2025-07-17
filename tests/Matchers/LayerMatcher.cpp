// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "LayerMatcher.hpp"

namespace {

testing::Matcher<odxf::Layer> IsLayer(const odxf::Layer& expected)
{
    return testing::AllOf(
        testing::Field("name", &odxf::Layer::name, expected.name),
        testing::Field("flags", &odxf::Layer::flags, expected.flags));
}

}   // namespace

testing::Matcher<std::vector<odxf::Layer>> AreLayers(const std::vector<odxf::Layer>& expected)
{
    std::vector<testing::Matcher<odxf::Layer>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::Layer& expectedLayer : expected) {
        elementMatchers.push_back(IsLayer(expectedLayer));
    }

    return testing::ElementsAreArray(elementMatchers);
}
