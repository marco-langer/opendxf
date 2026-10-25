// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "TablesMatcher.hpp"

#include "LayerMatcher.hpp"

#include <vector>

namespace {

testing::Matcher<odxf::LineTypeElement> IsLineTypeElement(const odxf::LineTypeElement& expected)
{
    return testing::AllOf(
        testing::Field(
            "length", &odxf::LineTypeElement::length, testing::DoubleNear(expected.length, 1e-9)),
        testing::Field(
            "complexFlags", &odxf::LineTypeElement::complexFlags, expected.complexFlags));
}

testing::Matcher<odxf::LineType> IsLineType(const odxf::LineType& expected)
{
    std::vector<testing::Matcher<odxf::LineTypeElement>> elementMatchers;
    elementMatchers.reserve(expected.elements.size());
    for (const odxf::LineTypeElement& element : expected.elements) {
        elementMatchers.push_back(IsLineTypeElement(element));
    }

    return testing::AllOf(
        testing::Field("name", &odxf::LineType::name, expected.name),
        testing::Field("displayName", &odxf::LineType::displayName, expected.displayName),
        testing::Field("flags", &odxf::LineType::flags, expected.flags),
        testing::Field(
            "elements",
            &odxf::LineType::elements,
            testing::ElementsAreArray(std::move(elementMatchers))));
}

testing::Matcher<odxf::LineTypes> AreLineTypes(const odxf::LineTypes& expected)
{
    std::vector<testing::Matcher<odxf::LineType>> elementMatchers;
    elementMatchers.reserve(expected.size());
    for (const odxf::LineType& lineType : expected) {
        elementMatchers.push_back(IsLineType(lineType));
    }

    return testing::ElementsAreArray(std::move(elementMatchers));
}

}   // namespace

testing::Matcher<odxf::Tables> AreTables(const odxf::Tables& expected)
{
    return testing::AllOf(
        testing::Field("lineTypes", &odxf::Tables::lineTypes, AreLineTypes(expected.lineTypes)),
        testing::Field("layers", &odxf::Tables::layers, AreLayers(expected.layers)));
}
