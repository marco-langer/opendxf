// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "opendxf/opendxf.hpp"

#include "test_utils.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <expected>
#include <filesystem>
#include <limits>
#include <optional>

TEST(insertion_units, valid)
{
    // Arrange
    const odxf::header header{ .entries{ { "$INSUNITS", 4 } } };

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertion_units(header)
    };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), odxf::length_unit::millimeters);
}

TEST(insertion_units, missing)
{
    // Arrange
    const odxf::header header{};

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertion_units(header)
    };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::nullopt);
}

TEST(insertion_units, wrong_type)
{
    // Arrange
    const odxf::header header{ .entries{ { "$INSUNITS", 4.0 } } };

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertion_units(header)
    };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::wrong_type);
}

class insertion_units_invalid_value_test : public testing::TestWithParam<int>
{};

TEST_P(insertion_units_invalid_value_test, invalid_value)
{
    // Arrange
    const odxf::header header{ .entries{ { "$INSUNITS", GetParam() } } };

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertion_units(header)
    };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::invalid_value);
}

INSTANTIATE_TEST_SUITE_P(
    insertion_units, insertion_units_invalid_value_test, testing::Values(-1, 25));

TEST(set_insertion_units, overwrites_existing_value)
{
    // Arrange
    odxf::header header{ .entries{ { "$INSUNITS", 4.0 } } };

    // Act
    odxf::set_insertion_units(header, odxf::length_unit::inches);

    // Assert
    EXPECT_EQ(odxf::insertion_units(header), odxf::length_unit::inches);
}

TEST(insertion_units, round_trip)
{
    // Arrange
    odxf::document document{ create_example_document() };
    odxf::set_insertion_units(document.header, odxf::length_unit::us_survey_feet);

    const std::filesystem::path file_path{ "insunits.dxf" };
    std::filesystem::remove(file_path);

    // Act
    const std::expected<void, odxf::error> write_result{ odxf::write(file_path, document) };

    const std::expected<odxf::document, odxf::error> read_document{ odxf::read(file_path) };

    // Assert
    ASSERT_TRUE(write_result.has_value());
    ASSERT_TRUE(read_document.has_value());
    EXPECT_EQ(odxf::insertion_units(read_document->header), odxf::length_unit::us_survey_feet);
}

TEST(line_type_scale, valid)
{
    // Arrange
    const odxf::header header{ .entries{ { "$LTSCALE", 2.5 } } };

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::line_type_scale(
        header) };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 2.5);
}

TEST(line_type_scale, missing)
{
    // Arrange
    const odxf::header header{};

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::line_type_scale(
        header) };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::nullopt);
}

TEST(line_type_scale, wrong_type)
{
    // Arrange
    const odxf::header header{ .entries{ { "$LTSCALE", 2 } } };

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::line_type_scale(
        header) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::wrong_type);
}

class line_type_scale_invalid_value_test : public testing::TestWithParam<double>
{};

TEST_P(line_type_scale_invalid_value_test, invalid_value)
{
    // Arrange
    const odxf::header header{ .entries{ { "$LTSCALE", GetParam() } } };

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::line_type_scale(
        header) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::invalid_value);
}

INSTANTIATE_TEST_SUITE_P(
    line_type_scale,
    line_type_scale_invalid_value_test,
    testing::Values(
        0.0,
        -1.0,
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()));

TEST(set_line_type_scale, overwrites_existing_value)
{
    // Arrange
    odxf::header header{ .entries{ { "$LTSCALE", 1.0 } } };

    // Act
    odxf::set_line_type_scale(header, 4.0);

    // Assert
    EXPECT_EQ(odxf::line_type_scale(header), 4.0);
}
