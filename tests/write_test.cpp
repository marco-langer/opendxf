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

std::vector<std::string> read_file(const std::filesystem::path& file_path)
{
    std::ifstream stream{ file_path };
    if (!stream.is_open()) {
        throw std::runtime_error{ std::format(
            "unable to open file '{}'",
            reinterpret_cast<const char*>(file_path.u8string().c_str())) };
    }

    std::vector<std::string> file_content;
    std::string line;
    while (std::getline(stream, line)) {
        file_content.push_back(line);
    }

    return file_content;
}

}   // namespace

TEST(Write, Example)
{
    // Arrange
    const odxf::document document{ create_example_document() };

    const std::filesystem::path file_path{ "test.dxf" };
    std::filesystem::remove(file_path);
    ASSERT_FALSE(std::filesystem::exists(file_path));

    // Act
    const std::expected<void, odxf::error> write_result{ odxf::write(file_path, document) };

    // Assert
    ASSERT_TRUE(write_result.has_value());
    ASSERT_TRUE(std::filesystem::is_regular_file(file_path));

    const std::expected<odxf::document, odxf::error> read_document{ odxf::read(file_path) };

    ASSERT_TRUE(read_document.has_value());
    EXPECT_THAT(*read_document, is_document(document));
}

TEST(Write, KeepsEntityOrder)
{
    // Arrange
    odxf::document document;
    document.entities = {
        odxf::circle{ .radius = 1.0 },          odxf::line{ .end = { 1.0, 0.0, 0.0 } },
        odxf::circle{ .radius = 2.0 },          odxf::arc{ .radius = 3.0, .end_angle = 90.0 },
        odxf::line{ .end = { 2.0, 0.0, 0.0 } },
    };

    const std::filesystem::path file_path{ "test_entity_order.dxf" };
    std::filesystem::remove(file_path);
    ASSERT_FALSE(std::filesystem::exists(file_path));

    // Act
    const std::expected<void, odxf::error> write_result{ odxf::write(file_path, document) };

    // Assert
    ASSERT_TRUE(write_result.has_value());
    const std::expected<odxf::document, odxf::error> read_document{ odxf::read(file_path) };

    ASSERT_TRUE(read_document.has_value());
    EXPECT_THAT(*read_document, is_document(document));
}

TEST(Write, LayerTransparencyRegistersApp)
{
    // Arrange
    odxf::document document;
    document.tables.layers.push_back(
        odxf::layer{ .name = "0", .transparency = odxf::transparency{ .alpha = 0x4C } });

    const std::filesystem::path file_path{ "test_appid.dxf" };
    std::filesystem::remove(file_path);
    ASSERT_FALSE(std::filesystem::exists(file_path));

    // Act
    const std::expected<void, odxf::error> write_result{ odxf::write(file_path, document) };

    // Assert
    ASSERT_TRUE(write_result.has_value());
    const std::vector<std::string> file_content{ read_file(file_path) };
    const std::vector<std::string> app_id_record{ "0", "APPID", "2", "AcCmTransparency" };

    EXPECT_FALSE(std::ranges::search(file_content, app_id_record).empty());
}

TEST(Write, NoAppIdsWithoutLayerTransparency)
{
    // Arrange
    odxf::document document;
    document.tables.layers.push_back(odxf::layer{ .name = "0" });

    const std::filesystem::path file_path{ "test_no_appid.dxf" };
    std::filesystem::remove(file_path);
    ASSERT_FALSE(std::filesystem::exists(file_path));

    // Act
    const std::expected<void, odxf::error> write_result{ odxf::write(file_path, document) };

    // Assert
    ASSERT_TRUE(write_result.has_value());
    const std::vector<std::string> file_content{ read_file(file_path) };

    EXPECT_THAT(file_content, testing::Not(testing::Contains("APPID")));
}

struct entity_type_param final
{
    odxf::entity entity;
    std::string type;
    std::string subclass;
};

struct EntityTypeFixture : testing::TestWithParam<entity_type_param>
{};

TEST_P(EntityTypeFixture, EntityType)
{
    // Arrange
    const auto& [entity, type, subclass]{ GetParam() };

    odxf::document document;
    document.entities = { entity };

    const std::filesystem::path file_path{ std::format("test_entity_type_{}.dxf", type) };
    std::filesystem::remove(file_path);
    ASSERT_FALSE(std::filesystem::exists(file_path));

    // Act
    const std::expected<void, odxf::error> write_result{ odxf::write(file_path, document) };

    // Assert
    ASSERT_TRUE(write_result.has_value());
    const std::vector<std::string> file_content{ read_file(file_path) };
    const std::vector<std::string> entity_begin{ "0", type, "100", subclass };

    EXPECT_FALSE(std::ranges::search(file_content, entity_begin).empty());
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    EntityTypeTest,
    EntityTypeFixture,
    testing::Values(
        entity_type_param{ .entity = odxf::arc{}, .type = "ARC", .subclass = "AcDbCircle" },
        entity_type_param{ .entity = odxf::circle{}, .type = "CIRCLE", .subclass = "AcDbCircle" },
        entity_type_param{ .entity = odxf::ellipse{}, .type = "ELLIPSE", .subclass = "AcDbEllipse" },
        entity_type_param{ .entity = odxf::line{}, .type = "LINE", .subclass = "AcDbLine" },
        entity_type_param{ .entity = odxf::lw_polyline{}, .type = "LWPOLYLINE", .subclass = "AcDbPolyline" },
        entity_type_param{ .entity = odxf::point{}, .type = "POINT", .subclass = "AcDbPoint" },
        entity_type_param{ .entity = odxf::ray{}, .type = "RAY", .subclass = "AcDbRay" }
    ),
    [](const testing::TestParamInfo<entity_type_param>& info) { return info.param.type; }
);
// clang-format on

TEST(Write, FileOpenError)
{
    // Arrange
    const std::filesystem::path file_path{ "missing_directory/test.dxf" };
    ASSERT_FALSE(std::filesystem::exists(file_path.parent_path()));

    // Act
    const std::expected<void, odxf::error> result{ odxf::write(file_path, odxf::document{}) };

    // Assert
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type, odxf::error_type::file_open_error);
}

TEST(WriteStream, EntityAfterClose)
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

TEST(WriteStream, CloseTwice)
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

TEST(WriteStream, DestructorCloses)
{
    // Arrange
    const std::filesystem::path file_path{ "test_destructor_closes.dxf" };
    std::filesystem::remove(file_path);
    ASSERT_FALSE(std::filesystem::exists(file_path));

    odxf::document document;
    document.entities = { odxf::line{ .end = { 1.0, 0.0, 0.0 } } };

    // Act
    {
        std::expected<odxf::write_stream, odxf::error> stream{ odxf::write_stream::open(
            file_path, document.header, document.tables) };
        ASSERT_TRUE(stream.has_value());
        ASSERT_TRUE(stream->entity(document.entities.front()).has_value());
    }

    // Assert
    const std::expected<odxf::document, odxf::error> read_document{ odxf::read(file_path) };

    ASSERT_TRUE(read_document.has_value());
    EXPECT_THAT(*read_document, is_document(document));
}
