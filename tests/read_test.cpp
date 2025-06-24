#include "opendxf/opendxf.hpp"

#include "Matchers/DocumentMatcher.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace {

class ReadStream : public odxf::IReadStream
{
public:
    const odxf::Document document() const { return m_document; }

    void header(const odxf::Header& header) override { m_document.header = header; }

private:
    odxf::Document m_document;
};

}   // namespace

TEST(read, minimalExample)
{
    // Arrange
    const auto filePath{ std::filesystem::path{ TEST_DATA_DIR } / "example.txt" };
    ASSERT_TRUE(std::filesystem::is_regular_file(filePath));

    ReadStream istream;

    // Act
    const tl::expected<void, odxf::Error> maybeError{ odxf::read(istream, filePath) };

    // Assert
    ASSERT_TRUE(maybeError.has_value());

    const odxf::Document& document{ istream.document() };

    odxf::Document expectedDocument;
    expectedDocument.header.entries.try_emplace("$ACADVER", "AC1032");

    EXPECT_THAT(document, IsDocument(expectedDocument));
}
