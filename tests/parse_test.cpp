// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "io/parse.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>

struct parse_int_param final
{
    std::string value;
    std::optional<int> expected;
};

struct ParseIntFixture : testing::TestWithParam<parse_int_param>
{};

TEST_P(ParseIntFixture, ParseAs)
{
    // Arrange
    const auto& [value, expected]{ GetParam() };

    // Act
    const std::optional<int> result{ odxf::parse_as<int>(value) };

    // Assert
    EXPECT_EQ(result, expected);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    ParseIntTest,
    ParseIntFixture,
    testing::Values(
        parse_int_param{ .value = "12", .expected = 12 },
        parse_int_param{ .value = "-12", .expected = -12 },
        parse_int_param{ .value = "    12", .expected = 12 },
        parse_int_param{ .value = "", .expected = std::nullopt },
        parse_int_param{ .value = "   ", .expected = std::nullopt },
        parse_int_param{ .value = "12abc", .expected = std::nullopt },
        parse_int_param{ .value = "12 ", .expected = std::nullopt },
        parse_int_param{ .value = "1.5", .expected = std::nullopt },
        parse_int_param{ .value = "99999999999", .expected = std::nullopt }
    )
);
// clang-format on

struct parse_double_param final
{
    std::string value;
    std::optional<double> expected;
};

struct ParseDoubleFixture : testing::TestWithParam<parse_double_param>
{};

TEST_P(ParseDoubleFixture, ParseAs)
{
    // Arrange
    const auto& [value, expected]{ GetParam() };

    // Act
    const std::optional<double> result{ odxf::parse_as<double>(value) };

    // Assert
    EXPECT_EQ(result, expected);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    ParseDoubleTest,
    ParseDoubleFixture,
    testing::Values(
        parse_double_param{ .value = "1.5", .expected = 1.5 },
        parse_double_param{ .value = "-0.5", .expected = -0.5 },
        parse_double_param{ .value = "  1.5", .expected = 1.5 },
        parse_double_param{ .value = "12", .expected = 12.0 },
        parse_double_param{ .value = ".5", .expected = 0.5 },
        parse_double_param{ .value = "5.", .expected = 5.0 },
        parse_double_param{ .value = "1.0E+20", .expected = 1.0e20 },
        parse_double_param{ .value = "", .expected = std::nullopt },
        parse_double_param{ .value = "1.234.567", .expected = std::nullopt },
        parse_double_param{ .value = "1.5 ", .expected = std::nullopt },
        parse_double_param{ .value = "1,5", .expected = std::nullopt },
        parse_double_param{ .value = "1e999", .expected = std::nullopt }
    )
);
// clang-format on
