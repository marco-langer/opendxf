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
    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "valid" / "example.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(filePath) };

    // Assert
    if (!result) {
        const odxf::error& error{ result.error() };
        FAIL() << std::format("Line ({}): {}", error.lineNumber.value_or(-1), error.what);
    }

    const odxf::document expectedDocument{ createExampleDocument() };

    EXPECT_THAT(*result, is_document(expectedDocument));
}

TEST(read, padded)
{
    // Arrange
    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "valid" / "padded.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(filePath) };

    // Assert
    EXPECT_TRUE(result.has_value());
    if (!result) {
        const odxf::error& error{ result.error() };
        FAIL() << std::format("Line ({}): {}", error.lineNumber.value_or(-1), error.what);
    }
}

TEST(read, minimalExample)
{
    GTEST_SKIP() << "reader requires a HEADER section";

    // Arrange
    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "valid" / "minimal_example.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(filePath) };

    // Assert
    if (!result) {
        const odxf::error& error{ result.error() };
        FAIL() << std::format("Line ({}): {}", error.lineNumber.value_or(-1), error.what);
    }
}

struct parse_error_param final
{
    std::string filename;
    int errorLine;
};

struct parse_error_fixture : testing::TestWithParam<parse_error_param>
{};

TEST_P(parse_error_fixture, parse_error)
{
    // Arrange
    const auto& [filename, expectedErrorLine]{ GetParam() };

    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "invalid" / filename };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    // Act
    const std::expected<odxf::document, odxf::error> result{ odxf::read(filePath) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().lineNumber, expectedErrorLine);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    parse_error_test,
    parse_error_fixture,
    testing::Values(
        parse_error_param{
            .filename = "missing_group_code.dxf",
            .errorLine = 5,
        },
        parse_error_param{
            .filename = "empty_group_code.dxf",
            .errorLine = 5,
        },
        parse_error_param{
            .filename = "empty_group_code_padded.dxf",
            .errorLine = 5,
        }
    )
);
// clang-format on
