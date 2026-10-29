// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "header_matcher.hpp"

#include "coordinate_matcher.hpp"
#include "overload.hpp"

namespace {

testing::Matcher<odxf::header_entries>
are_header_entries(const odxf::header_entries& expected, double max_error)
{
    std::vector<testing::Matcher<std::pair<const odxf::header_key, odxf::header_value>>>
        element_matchers;
    element_matchers.reserve(expected.size());

    for (const auto& [key, variant_value] : expected) {
        std::visit(
            odxf::overload{
                [&](bool value) {
                    element_matchers.push_back(
                        testing::Pair(key, testing::VariantWith<bool>(testing::Eq(value))));
                },
                [&](int value) {
                    element_matchers.push_back(
                        testing::Pair(key, testing::VariantWith<int>(testing::Eq(value))));
                },
                [&](double value) {
                    element_matchers.push_back(
                        testing::Pair(key, testing::VariantWith<double>(testing::Eq(value))));
                },
                [&](const std::string& value) {
                    element_matchers.push_back(
                        testing::Pair(key, testing::VariantWith<std::string>(testing::Eq(value))));
                },
                [&](const odxf::coordinate2d& value) {
                    element_matchers.push_back(
                        testing::Pair(
                            key,
                            testing::VariantWith<odxf::coordinate2d>(
                                is_coordinate(value, max_error))));
                },
                [&](const odxf::coordinate3d& value) {
                    element_matchers.push_back(
                        testing::Pair(
                            key,
                            testing::VariantWith<odxf::coordinate3d>(
                                is_coordinate(value, max_error))));
                } },
            variant_value);
    }

    return testing::UnorderedElementsAreArray(element_matchers);
}

}   // namespace

testing::Matcher<odxf::header> is_header(const odxf::header& expected, double max_error)
{
    return testing::Field(
        "entries", &odxf::header::entries, are_header_entries(expected.entries, max_error));
}
