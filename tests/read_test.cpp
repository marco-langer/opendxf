// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/opendxf.hpp"

#include "Matchers/DocumentMatcher.hpp"
#include "TestUtils.hpp"

#include <fmt/format.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace {

class ReadStream final : public odxf::IReadStream
{
public:
    const odxf::Document& document() const& { return m_document; }

private:
    void header(const odxf::Header& header) override { m_document.header = header; }
    void layer(const odxf::Layer& layer) override { m_document.tables.layers.push_back(layer); }

    void arc(const odxf::Arc& arc) override { m_document.entities.arcs.push_back(arc); }
    void circle(const odxf::Circle& circle) override
    {
        m_document.entities.circles.push_back(circle);
    }
    void line(const odxf::Line& line) override { m_document.entities.lines.push_back(line); }
    void lwPolyline(const odxf::LWPolyline& lwPolyline) override
    {
        m_document.entities.lwPolylines.push_back(lwPolyline);
    }

    odxf::Document m_document;
};

}   // namespace

TEST(read, example)
{
    // Arrange
    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "example.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    ReadStream istream;

    // Act
    const tl::expected<void, odxf::Error> result{ odxf::read(istream, filePath) };

    // Assert
    if (!result) {
        const odxf::Error& error{ result.error() };
        FAIL() << fmt::format("Line ({}): {}", error.lineNumber.value_or(-1), error.what);
    }

    const odxf::Document& document{ istream.document() };

    const odxf::Document expectedDocument{ createExampleDocument() };

    EXPECT_THAT(document, IsDocument(expectedDocument));
}

TEST(read, padded)
{
    // Arrange
    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "padded.dxf" };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    ReadStream istream;

    // Act
    const tl::expected<void, odxf::Error> result{ odxf::read(istream, filePath) };

    // Assert
    EXPECT_TRUE(result.has_value());
    if (!result) {
        const odxf::Error& error{ result.error() };
        FAIL() << fmt::format("Line ({}): {}", error.lineNumber.value_or(-1), error.what);
    }
}

struct ParseErrorParam final
{
    std::string filename;
    int errorLine;
};

struct ParseErrorFixture : testing::TestWithParam<ParseErrorParam>
{};

TEST_P(ParseErrorFixture, ParseError)
{
    // Arrange
    const auto& [filename, expectedErrorLine]{ GetParam() };

    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / filename };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    ReadStream istream;

    // Act
    const tl::expected<void, odxf::Error> result{ odxf::read(istream, filePath) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().lineNumber, expectedErrorLine);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    ParseErrorTest,
    ParseErrorFixture,
    testing::Values(
        ParseErrorParam{
            .filename = "missing_group_code.dxf",
            .errorLine = 5,
        },
        ParseErrorParam{
            .filename = "empty_group_code.dxf",
            .errorLine = 5,
        },
        ParseErrorParam{
            .filename = "empty_group_code_padded.dxf",
            .errorLine = 5,
        }
    )
);
// clang-format on
