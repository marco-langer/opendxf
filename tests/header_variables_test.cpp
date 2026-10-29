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

TEST(InsertionUnits, Valid)
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

TEST(InsertionUnits, Missing)
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

TEST(InsertionUnits, WrongType)
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

class InsertionUnitsInvalidValueTest : public testing::TestWithParam<int>
{};

TEST_P(InsertionUnitsInvalidValueTest, InvalidValue)
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

INSTANTIATE_TEST_SUITE_P(InsertionUnits, InsertionUnitsInvalidValueTest, testing::Values(-1, 25));

TEST(SetInsertionUnits, OverwritesExistingValue)
{
    // Arrange
    odxf::header header{ .entries{ { "$INSUNITS", 4.0 } } };

    // Act
    odxf::set_insertion_units(header, odxf::length_unit::inches);

    // Assert
    EXPECT_EQ(odxf::insertion_units(header), odxf::length_unit::inches);
}

TEST(InsertionUnits, RoundTrip)
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

TEST(LineTypeScale, Valid)
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

TEST(LineTypeScale, Missing)
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

TEST(LineTypeScale, WrongType)
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

class LineTypeScaleInvalidValueTest : public testing::TestWithParam<double>
{};

TEST_P(LineTypeScaleInvalidValueTest, InvalidValue)
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
    LineTypeScale,
    LineTypeScaleInvalidValueTest,
    testing::Values(
        0.0,
        -1.0,
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()));

TEST(SetLineTypeScale, OverwritesExistingValue)
{
    // Arrange
    odxf::header header{ .entries{ { "$LTSCALE", 1.0 } } };

    // Act
    odxf::set_line_type_scale(header, 4.0);

    // Assert
    EXPECT_EQ(odxf::line_type_scale(header), 4.0);
}
