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

private:
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
    const tl::expected<void, odxf::Error> maybeError{ odxf::read(istream, filePath) };

    // Assert
    if (!maybeError) {
        const odxf::Error& error{ maybeError.error() };
        FAIL() << fmt::format("Line ({}): {}", error.lineNumber.value_or(-1), error.what);
    }

    const odxf::Document& document{ istream.document() };

    const odxf::Document expectedDocument{ createExampleDocument() };

    EXPECT_THAT(document, IsDocument(expectedDocument));
}
