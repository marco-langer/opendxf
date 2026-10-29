// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/read.hpp"
#include "opendxf/io/write.hpp"

#include "Matchers/DocumentMatcher.hpp"
#include "TestUtils.hpp"

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
    const odxf::Document document{ createExampleDocument() };

    const std::filesystem::path filePath{ "test.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::Error> writeResult{ odxf::write(filePath, document) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    const std::expected<odxf::Document, odxf::Error> readDocument{ odxf::read(filePath) };

    ASSERT_TRUE(readDocument.has_value());
    EXPECT_THAT(*readDocument, IsDocument(document));
}

TEST(write, keepsEntityOrder)
{
    // Arrange
    odxf::Document document;
    document.entities = {
        odxf::Circle{ .radius = 1.0 },          odxf::Line{ .end = { 1.0, 0.0, 0.0 } },
        odxf::Circle{ .radius = 2.0 },          odxf::Arc{ .radius = 3.0, .endAngle = 90.0 },
        odxf::Line{ .end = { 2.0, 0.0, 0.0 } },
    };

    const std::filesystem::path filePath{ "test_entity_order.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::Error> writeResult{ odxf::write(filePath, document) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    const std::expected<odxf::Document, odxf::Error> readDocument{ odxf::read(filePath) };

    ASSERT_TRUE(readDocument.has_value());
    EXPECT_THAT(*readDocument, IsDocument(document));
}

TEST(write, layerTransparencyRegistersApp)
{
    // Arrange
    odxf::Document document;
    document.tables.layers.push_back(
        odxf::Layer{ .name = "0", .transparency = odxf::Transparency{ .alpha = 0x4C } });

    const std::filesystem::path filePath{ "test_appid.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::Error> writeResult{ odxf::write(filePath, document) };

    // Assert
    ASSERT_TRUE(writeResult.has_value());
    const std::vector<std::string> fileContent{ readFile(filePath) };
    const std::vector<std::string> appIdRecord{ "0", "APPID", "2", "AcCmTransparency" };

    EXPECT_FALSE(std::ranges::search(fileContent, appIdRecord).empty());
}

TEST(write, noAppIdsWithoutLayerTransparency)
{
    // Arrange
    odxf::Document document;
    document.tables.layers.push_back(odxf::Layer{ .name = "0" });

    const std::filesystem::path filePath{ "test_no_appid.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    // Act
    const std::expected<void, odxf::Error> writeResult{ odxf::write(filePath, document) };

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
    const std::expected<void, odxf::Error> result{ odxf::write(filePath, odxf::Document{}) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type, odxf::Error::Type::FileOpenError);
}

TEST(writeStream, entityAfterClose)
{
    // Arrange
    std::expected<odxf::WriteStream, odxf::Error> stream{ odxf::WriteStream::open(
        "test_entity_after_close.dxf", odxf::Header{}, odxf::Tables{}) };
    ASSERT_TRUE(stream.has_value());
    ASSERT_TRUE(stream->close().has_value());

    // Act
    const std::expected<void, odxf::Error> result{ stream->entity(odxf::Line{}) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type, odxf::Error::Type::WriteError);
}

TEST(writeStream, closeTwice)
{
    // Arrange
    std::expected<odxf::WriteStream, odxf::Error> stream{ odxf::WriteStream::open(
        "test_close_twice.dxf", odxf::Header{}, odxf::Tables{}) };
    ASSERT_TRUE(stream.has_value());
    ASSERT_TRUE(stream->close().has_value());

    // Act
    const std::expected<void, odxf::Error> result{ stream->close() };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type, odxf::Error::Type::WriteError);
}

TEST(writeStream, destructorCloses)
{
    // Arrange
    const std::filesystem::path filePath{ "test_destructor_closes.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    odxf::Document document;
    document.entities = { odxf::Line{ .end = { 1.0, 0.0, 0.0 } } };

    // Act
    {
        std::expected<odxf::WriteStream, odxf::Error> stream{ odxf::WriteStream::open(
            filePath, document.header, document.tables) };
        ASSERT_TRUE(stream.has_value());
        ASSERT_TRUE(stream->entity(document.entities.front()).has_value());
    }

    // Assert
    const std::expected<odxf::Document, odxf::Error> readDocument{ odxf::read(filePath) };

    ASSERT_TRUE(readDocument.has_value());
    EXPECT_THAT(*readDocument, IsDocument(document));
}
