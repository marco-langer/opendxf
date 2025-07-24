// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "HeaderMatcher.hpp"

#include "CoordinateMatcher.hpp"

namespace {

template <typename... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};

testing::Matcher<std::unordered_map<odxf::HeaderKey, odxf::HeaderValue>> AreHeaderEntries(
    const std::unordered_map<odxf::HeaderKey, odxf::HeaderValue>& expected, double maxError)
{
    std::vector<testing::Matcher<std::pair<const odxf::HeaderKey, odxf::HeaderValue>>>
        elementMatchers;
    elementMatchers.reserve(expected.size());

    for (const auto& [key, variantValue] : expected) {
        std::visit(
            overload{
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
                [&](const odxf::Coordinate2d& value) {
                    elementMatchers.push_back(testing::Pair(
                        key,
                        testing::VariantWith<odxf::Coordinate2d>(IsCoordinate(value, maxError))));
                },
                [&](const odxf::Coordinate3d& value) {
                    elementMatchers.push_back(testing::Pair(
                        key,
                        testing::VariantWith<odxf::Coordinate3d>(IsCoordinate(value, maxError))));
                } },
            variantValue);
    }

    return testing::UnorderedElementsAreArray(elementMatchers);
}

}   // namespace

testing::Matcher<odxf::Header> IsHeader(const odxf::Header& expected, double maxError)
{
    return testing::Field(
        "entries", &odxf::Header::entries, AreHeaderEntries(expected.entries, maxError));
}
