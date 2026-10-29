// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/opendxf.hpp"

#include "matchers/document_matcher.hpp"
#include "test_utils.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <format>

TEST(read, example)
{
    // Arrange
    const auto file_path{ std::filesystem::path{ TEST_DATA_DIR } / "valid" / "example.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(file_path));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(file_path) };

    // Assert
    if (!result) {
        const odxf::error& error{ result.error() };
        FAIL() << std::format("Line ({}): {}", error.line_number.value_or(-1), error.what);
    }

    const odxf::document expected_document{ create_example_document() };

    EXPECT_THAT(*result, is_document(expected_document));
}

TEST(read, padded)
{
    // Arrange
    const auto file_path{ std::filesystem::path{ TEST_DATA_DIR } / "valid" / "padded.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(file_path));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(file_path) };

    // Assert
    EXPECT_TRUE(result.has_value());
    if (!result) {
        const odxf::error& error{ result.error() };
        FAIL() << std::format("Line ({}): {}", error.line_number.value_or(-1), error.what);
    }
}

TEST(read, minimal_example)
{
    GTEST_SKIP() << "reader requires a HEADER section";

    // Arrange
    const auto file_path{ std::filesystem::path{ TEST_DATA_DIR } / "valid"
                          / "minimal_example.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(file_path));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(file_path) };

    // Assert
    if (!result) {
        const odxf::error& error{ result.error() };
        FAIL() << std::format("Line ({}): {}", error.line_number.value_or(-1), error.what);
    }
}

struct parse_error_param final
{
    std::string filename;
    int error_line;
};

struct parse_error_fixture : testing::TestWithParam<parse_error_param>
{};

TEST_P(parse_error_fixture, parse_error)
{
    // Arrange
    const auto& [filename, expected_error_line]{ GetParam() };

    const auto file_path{ std::filesystem::path{ TEST_DATA_DIR } / "invalid" / filename };
    ASSERT_TRUE(std::filesystem::is_regular_file(file_path));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(file_path) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().line_number, expected_error_line);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    parse_error_test,
    parse_error_fixture,
    testing::Values(
        parse_error_param{
            .filename = "missing_group_code.dxf",
            .error_line = 5,
        },
        parse_error_param{
            .filename = "empty_group_code.dxf",
            .error_line = 5,
        },
        parse_error_param{
            .filename = "empty_group_code_padded.dxf",
            .error_line = 5,
        }
    )
);
// clang-format on
