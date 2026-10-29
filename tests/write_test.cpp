// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/read.hpp"
#include "opendxf/io/write.hpp"

#include "matchers/document_matcher.hpp"
#include "test_utils.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> readFile(const std::filesystem::path& filePath)
{
    std::ifstream stream{ filePath };
    if (!stream.is_open()) {
        throw std::runtime_error{ std::format(
            "unable to open file '{}'",
            reinterpret_cast<const char*>(filePath.u8string().c_str())) };
    }

    std::vector<std::string> fileContent;
    std::string line;
    while (std::getline(stream, line)) {
        fileContent.push_back(line);
    }

    return fileContent;
}

}   // namespace

TEST(write, example)
{
    // Arrange
    const odxf::document document{ createExampleDocument() };

    const std::filesystem::path filePath{ "test.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::error> writeResult{ odxf::write(filePath, document) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    const std::expected<odxf::document, odxf::error> readDocument{ odxf::read(filePath) };

    ASSERT_TRUE(readDocument.has_value());
    EXPECT_THAT(*readDocument, is_document(document));
}

TEST(write, keepsEntityOrder)
{
    // Arrange
    odxf::document document;
    document.entities = {
        odxf::circle{ .radius = 1.0 },          odxf::line{ .end = { 1.0, 0.0, 0.0 } },
        odxf::circle{ .radius = 2.0 },          odxf::arc{ .radius = 3.0, .endAngle = 90.0 },
        odxf::line{ .end = { 2.0, 0.0, 0.0 } },
    };

    const std::filesystem::path filePath{ "test_entity_order.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::error> writeResult{ odxf::write(filePath, document) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    const std::expected<odxf::document, odxf::error> readDocument{ odxf::read(filePath) };

    ASSERT_TRUE(readDocument.has_value());
    EXPECT_THAT(*readDocument, is_document(document));
}

TEST(write, layerTransparencyRegistersApp)
{
    // Arrange
    odxf::document document;
    document.tables.layers.push_back(
        odxf::layer{ .name = "0", .transparency = odxf::transparency{ .alpha = 0x4C } });

    const std::filesystem::path filePath{ "test_appid.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::error> writeResult{ odxf::write(filePath, document) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    const std::vector<std::string> fileContent{ readFile(filePath) };
    const std::vector<std::string> appIdRecord{ "0", "APPID", "2", "AcCmTransparency" };

    EXPECT_FALSE(std::ranges::search(fileContent, appIdRecord).empty());
}

TEST(write, noAppIdsWithoutLayerTransparency)
{
    // Arrange
    odxf::document document;
    document.tables.layers.push_back(odxf::layer{ .name = "0" });

    const std::filesystem::path filePath{ "test_no_appid.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::error> writeResult{ odxf::write(filePath, document) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    const std::vector<std::string> fileContent{ readFile(filePath) };

    EXPECT_THAT(fileContent, testing::Not(testing::Contains("APPID")));
}

TEST(write, fileOpenError)
{
    // Arrange
    const std::filesystem::path filePath{ "missing_directory/test.dxf" };
    ASSERT_FALSE(std::filesystem::exists(filePath.parent_path()));

    // Act
    const std::expected<void, odxf::error> result{ odxf::write(filePath, odxf::document{}) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type, odxf::error_type::file_open_error);
}

TEST(writeStream, entityAfterClose)
{
    // Arrange
    std::expected<odxf::write_stream, odxf::error> stream{ odxf::write_stream::open(
        "test_entity_after_close.dxf", odxf::header{}, odxf::tables{}) };
    ASSERT_TRUE(stream.has_value());
    ASSERT_TRUE(stream->close().has_value());

    // Act
    const std::expected<void, odxf::error> result{ stream->entity(odxf::line{}) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type, odxf::error_type::write_error);
}

TEST(writeStream, closeTwice)
{
    // Arrange
    std::expected<odxf::write_stream, odxf::error> stream{ odxf::write_stream::open(
        "test_close_twice.dxf", odxf::header{}, odxf::tables{}) };
    ASSERT_TRUE(stream.has_value());
    ASSERT_TRUE(stream->close().has_value());

    // Act
    const std::expected<void, odxf::error> result{ stream->close() };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type, odxf::error_type::write_error);
}

TEST(writeStream, destructorCloses)
{
    // Arrange
    const std::filesystem::path filePath{ "test_destructor_closes.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    odxf::document document;
    document.entities = { odxf::line{ .end = { 1.0, 0.0, 0.0 } } };

    // Act
    {
        std::expected<odxf::write_stream, odxf::error> stream{ odxf::write_stream::open(
            filePath, document.header, document.tables) };
        ASSERT_TRUE(stream.has_value());
        ASSERT_TRUE(stream->entity(document.entities.front()).has_value());
    }

    // Assert
    const std::expected<odxf::document, odxf::error> readDocument{ odxf::read(filePath) };

    ASSERT_TRUE(readDocument.has_value());
    EXPECT_THAT(*readDocument, is_document(document));
}
