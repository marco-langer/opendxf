// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/read.hpp"
#include "opendxf/write.hpp"

#include "Matchers/DocumentMatcher.hpp"
#include "TestUtils.hpp"

#include <fmt/core.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> readFile(const std::filesystem::path& filePath)
{
    std::ifstream stream{ filePath };
    if (!stream.is_open()) {
        throw std::runtime_error{ fmt::format(
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
    const tl::expected<void, odxf::Error> result{ odxf::read(istream, filePath) };

    ASSERT_TRUE(result.has_value());

    const odxf::Document& readDocument{ istream.document() };

    EXPECT_THAT(readDocument, IsDocument(document));
}
