// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "header_matcher.hpp"

#include "coordinate_matcher.hpp"
#include "overload.hpp"

namespace {

testing::Matcher<odxf::header_entries>
are_header_entries(const odxf::header_entries& expected, double maxError)
{
    std::vector<testing::Matcher<std::pair<const odxf::header_key, odxf::header_value>>>
        elementMatchers;
    elementMatchers.reserve(expected.size());

    for (const auto& [key, variantValue] : expected) {
        std::visit(
            odxf::overload{
                [&](bool value) {
                    elementMatchers.push_back(
                        testing::Pair(key, testing::VariantWith<bool>(testing::Eq(value))));
                },
                [&](int value) {
                    elementMatchers.push_back(
                        testing::Pair(key, testing::VariantWith<int>(testing::Eq(value))));
                },
                [&](double value) {
                    elementMatchers.push_back(
                        testing::Pair(key, testing::VariantWith<double>(testing::Eq(value))));
                },
                [&](const std::string& value) {
                    elementMatchers.push_back(
                        testing::Pair(key, testing::VariantWith<std::string>(testing::Eq(value))));
                },
                [&](const odxf::coordinate2d& value) {
                    elementMatchers.push_back(
                        testing::Pair(
                            key,
                            testing::VariantWith<odxf::coordinate2d>(
                                is_coordinate(value, maxError))));
                },
                [&](const odxf::coordinate3d& value) {
                    elementMatchers.push_back(
                        testing::Pair(
                            key,
                            testing::VariantWith<odxf::coordinate3d>(
                                is_coordinate(value, maxError))));
                } },
            variantValue);
    }

    return testing::UnorderedElementsAreArray(elementMatchers);
}

}   // namespace

testing::Matcher<odxf::header> is_header(const odxf::header& expected, double maxError)
{
    return testing::Field(
        "entries", &odxf::header::entries, are_header_entries(expected.entries, maxError));
}
