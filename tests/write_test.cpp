#include "opendxf/write.hpp"

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

    const auto expectedFilePath{ std::filesystem::path{ TEST_DATA_DIR } / "example.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(expectedFilePath));

    EXPECT_THAT(readFile(filePath), testing::ElementsAreArray(readFile(expectedFilePath)));
}
