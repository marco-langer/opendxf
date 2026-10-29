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

TEST(insertionUnits, valid)
{
    // Arrange
    const odxf::header header{ .entries{ { "$INSUNITS", 4 } } };

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), odxf::length_unit::millimeters);
}

TEST(insertionUnits, missing)
{
    // Arrange
    const odxf::header header{};

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::nullopt);
}

TEST(insertionUnits, wrongType)
{
    // Arrange
    const odxf::header header{ .entries{ { "$INSUNITS", 4.0 } } };

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::wrong_type);
}

class insertion_units_invalid_value_test : public testing::TestWithParam<int>
{};

TEST_P(insertion_units_invalid_value_test, invalidValue)
{
    // Arrange
    const odxf::header header{ .entries{ { "$INSUNITS", GetParam() } } };

    // Act
    const std::expected<std::optional<odxf::length_unit>, odxf::header_error> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::invalid_value);
}

INSTANTIATE_TEST_SUITE_P(
    insertionUnits, insertion_units_invalid_value_test, testing::Values(-1, 25));

TEST(setInsertionUnits, overwritesExistingValue)
{
    // Arrange
    odxf::header header{ .entries{ { "$INSUNITS", 4.0 } } };

    // Act
    odxf::setInsertionUnits(header, odxf::length_unit::inches);

    // Assert
    EXPECT_EQ(odxf::insertionUnits(header), odxf::length_unit::inches);
}

TEST(insertionUnits, roundTrip)
{
    // Arrange
    odxf::document document{ createExampleDocument() };
    odxf::setInsertionUnits(document.header, odxf::length_unit::us_survey_feet);

    const std::filesystem::path filePath{ "insunits.dxf" };
    std::filesystem::remove(filePath);

    // Act
    const std::expected<void, odxf::error> writeResult{ odxf::write(filePath, document) };

    const std::expected<odxf::document, odxf::error> readDocument{ odxf::read(filePath) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    ASSERT_TRUE(readDocument.has_value());
    EXPECT_EQ(odxf::insertionUnits(readDocument->header), odxf::length_unit::us_survey_feet);
}

TEST(lineTypeScale, valid)
{
    // Arrange
    const odxf::header header{ .entries{ { "$LTSCALE", 2.5 } } };

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::lineTypeScale(
        header) };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 2.5);
}

TEST(lineTypeScale, missing)
{
    // Arrange
    const odxf::header header{};

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::lineTypeScale(
        header) };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::nullopt);
}

TEST(lineTypeScale, wrongType)
{
    // Arrange
    const odxf::header header{ .entries{ { "$LTSCALE", 2 } } };

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::lineTypeScale(
        header) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::wrong_type);
}

class line_type_scale_invalid_value_test : public testing::TestWithParam<double>
{};

TEST_P(line_type_scale_invalid_value_test, invalidValue)
{
    // Arrange
    const odxf::header header{ .entries{ { "$LTSCALE", GetParam() } } };

    // Act
    const std::expected<std::optional<double>, odxf::header_error> result{ odxf::lineTypeScale(
        header) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::header_error::invalid_value);
}

INSTANTIATE_TEST_SUITE_P(
    lineTypeScale,
    line_type_scale_invalid_value_test,
    testing::Values(
        0.0,
        -1.0,
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()));

TEST(setLineTypeScale, overwritesExistingValue)
{
    // Arrange
    odxf::header header{ .entries{ { "$LTSCALE", 1.0 } } };

    // Act
    odxf::setLineTypeScale(header, 4.0);

    // Assert
    EXPECT_EQ(odxf::lineTypeScale(header), 4.0);
}
