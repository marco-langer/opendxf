// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "opendxf/opendxf.hpp"

#include "TestUtils.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <expected>
#include <filesystem>
#include <optional>

TEST(insertionUnits, valid)
{
    // Arrange
    const odxf::Header header{ .entries{ { "$INSUNITS", 4 } } };

    // Act
    const std::expected<std::optional<odxf::LengthUnit>, odxf::HeaderError> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), odxf::LengthUnit::Millimeters);
}

TEST(insertionUnits, missing)
{
    // Arrange
    const odxf::Header header{};

    // Act
    const std::expected<std::optional<odxf::LengthUnit>, odxf::HeaderError> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::nullopt);
}

TEST(insertionUnits, wrongType)
{
    // Arrange
    const odxf::Header header{ .entries{ { "$INSUNITS", 4.0 } } };

    // Act
    const std::expected<std::optional<odxf::LengthUnit>, odxf::HeaderError> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::HeaderError::WrongType);
}

class InsertionUnitsInvalidValueTest : public testing::TestWithParam<int>
{};

TEST_P(InsertionUnitsInvalidValueTest, invalidValue)
{
    // Arrange
    const odxf::Header header{ .entries{ { "$INSUNITS", GetParam() } } };

    // Act
    const std::expected<std::optional<odxf::LengthUnit>, odxf::HeaderError> result{
        odxf::insertionUnits(header)
    };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), odxf::HeaderError::InvalidValue);
}

INSTANTIATE_TEST_SUITE_P(insertionUnits, InsertionUnitsInvalidValueTest, testing::Values(-1, 25));

TEST(setInsertionUnits, overwritesExistingValue)
{
    // Arrange
    odxf::Header header{ .entries{ { "$INSUNITS", 4.0 } } };

    // Act
    odxf::setInsertionUnits(header, odxf::LengthUnit::Inches);

    // Assert
    EXPECT_EQ(odxf::insertionUnits(header), odxf::LengthUnit::Inches);
}

TEST(insertionUnits, roundTrip)
{
    // Arrange
    odxf::Document document{ createExampleDocument() };
    odxf::setInsertionUnits(document.header, odxf::LengthUnit::USSurveyFeet);

    const std::filesystem::path filePath{ "insunits.dxf" };
    std::filesystem::remove(filePath);

    // Act
    odxf::writeDxf(document, filePath);

    ReadStream istream;
    const std::expected<void, odxf::Error> result{ odxf::read(istream, filePath) };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(odxf::insertionUnits(istream.document().header), odxf::LengthUnit::USSurveyFeet);
}

TEST(insertionUnits, readFromFile)
{
    // Arrange
    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "valid" / "example.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    ReadStream istream;

    // Act
    const std::expected<void, odxf::Error> result{ odxf::read(istream, filePath) };

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(odxf::insertionUnits(istream.document().header), odxf::LengthUnit::Millimeters);
}
