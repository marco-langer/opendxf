#include "opendxf/opendxf.hpp"

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
    const auto expectedFilePath{ std::filesystem::path{ TEST_DATA_DIR } / "example.txt" };
    ASSERT_TRUE(std::filesystem::is_regular_file(expectedFilePath));

    odxf::Document document{
        .header{ .entries{ { "$ACADVER", "AC1032" } } },
        .tables{ .lineTypes{ { .name{ "CONTINUOUS" }, .displayName{ "Solid Line" } } },
                 .layers{ { .name{ "Test Layer" } } } },
        .entities{ .lines{ { .start{ 0.0F, 0.5F, 0.0F }, .end{ 1.0F, 1.5F, 0.0F } },
                           { .end{ 1.0F, 1.0F, 0.0F } } },
                   .circles{ { .center{}, .radius{ 1.0F } } },
                   .arcs{ { .center{ 0.0F, 2.0F },
                            .radius{ 0.5F },
                            .startAngle{ 0.0F },
                            .endAngle{ 180.0F } } } }
    };

    const std::filesystem::path filePath{ "test.dxf" };
    std::filesystem::remove(filePath);
    ASSERT_FALSE(std::filesystem::exists(filePath));

    odxf::writeDxf(document, "test.dxf");

    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    EXPECT_THAT(readFile(filePath), testing::ElementsAreArray(readFile(expectedFilePath)));
}
