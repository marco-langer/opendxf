#include "opendxf/opendxf.hpp"

#include "Matchers/DocumentMatcher.hpp"

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
        FAIL() << maybeError.error().what;
    }

    const odxf::Document& document{ istream.document() };

    odxf::Document expectedDocument;
    expectedDocument.header.entries.try_emplace("$ACADVER", "AC1032");

    const std::string layerName1{ "Test Layer" };
    odxf::Layers& layers{ expectedDocument.tables.layers };
    layers.push_back(odxf::Layer{ .name = layerName1 });

    odxf::Lines& lines{ expectedDocument.entities.lines };
    lines
        .emplace_back(odxf::Line{
            .start = odxf::Coordinate3d{ 0.0, 0.5, 0.0 },
            .end = odxf::Coordinate3d{ 1.0, 1.5, 0.0 },
        })
        .layer = layerName1;

    lines
        .emplace_back(odxf::Line{
            .start = odxf::Coordinate3d{ 0.0, 0.0, 0.0 },
            .end = odxf::Coordinate3d{ 1.0, 1.0, 0.0 },
        })
        .layer = layerName1;

    odxf::Circles& circles{ expectedDocument.entities.circles };
    circles
        .emplace_back(odxf::Circle{ .center = odxf::Coordinate3d{ 0.0, 0.0, 0.0 }, .radius = 1.0F })
        .layer = layerName1;

    odxf::Arcs& arcs{ expectedDocument.entities.arcs };
    arcs.emplace_back(odxf::Arc{
                          .center = odxf::Coordinate3d{ 0.0F, 2.0F, 0.0F },
                          .radius = 0.5F,
                          .startAngle = 0.0F,
                          .endAngle = 180.0F,
                      })
        .layer = layerName1;

    EXPECT_THAT(document, IsDocument(expectedDocument));
}
