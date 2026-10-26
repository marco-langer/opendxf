// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/read.hpp"
#include "opendxf/write.hpp"

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
    odxf::writeDxf(document, filePath);

    // Assert
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    ReadStream istream;
    const std::expected<void, odxf::Error> result{ odxf::read(istream, filePath) };

    ASSERT_TRUE(result.has_value());

    const odxf::Document& readDocument{ istream.document() };

    EXPECT_THAT(readDocument, IsDocument(document));
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
    odxf::writeDxf(document, filePath);

    // Assert
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
    odxf::writeDxf(document, filePath);

    // Assert
    const std::vector<std::string> fileContent{ readFile(filePath) };

    EXPECT_THAT(fileContent, testing::Not(testing::Contains("APPID")));
}
