// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/io/error.hpp"

#include <concepts>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace odxf {

class read_stream_base;

// A type that holds the properties all entities share, e.g. Line.
template <typename E>
concept entity_type = requires(E& entity) {
    { entity.properties } -> std::same_as<entity_properties&>;
};

class reader
{
public:
    reader(read_stream_base& stream);

    reader(const reader&) = delete;
    reader(reader&&) = delete;
    reader& operator=(const reader&) = delete;
    reader& operator=(reader&&) = delete;

    std::expected<void, error> readAll(const std::filesystem::path& filePath);

private:
    bool open(const std::filesystem::path& filePath);

    std::expected<void, error> readHeader();
    std::expected<header_entry, error> readHeaderEntry();
    std::expected<std::variant<coordinate2d, coordinate3d>, error> readHeaderCoordinate();

    std::expected<void, error> readTables();
    std::expected<void, error> readLineType();
    std::expected<void, error> readLayer();

    std::expected<void, error> readBlocks();

    std::expected<void, error> readEntities();
    std::expected<void, error> readLine();
    std::expected<void, error> readCircle();
    std::expected<void, error> readArc();
    std::expected<void, error> readLWPolyline();

    // Reads the group codes of one entity up to the next group code 0. readGroupCode is called for
    // each group code and returns false on an error.
    template <entity_type E, std::predicate<E&> F>
    std::expected<E, error> readEntity(F readGroupCode);
    // Reads the group codes all entities share, skips unknown ones. Returns false on an error.
    bool readEntityProperty(entity_properties& properties);

    bool readNext();
    bool readNextSingle();

    // parse the current value into target, on failure sets the error and returns false
    template <typename T>
    bool parseInto(T& target);
    template <typename T>
    bool parseInto(std::optional<T>& target);

    bool hasError() const;
    std::expected<void, error> makeError() const;
    // an Error at the current line, without a description
    std::unexpected<error> parseError() const;
    bool isSectionBegin() const;
    bool isSectionEnd() const;
    bool isHeaderBegin() const;
    bool isTablesBegin() const;
    bool isBlocksBegin() const;
    bool isEntitiesBegin() const;
    bool isTableBegin() const;
    bool isTableEnd() const;
    bool isLayerBegin() const;
    bool isLayer() const;
    bool isLineType() const;

    bool isLine() const;
    bool isCircle() const;
    bool isArc() const;
    bool isLWPolyline() const;

    bool isEOF() const;

    struct data
    {
        int groupCode;
        std::string value;
    };

    read_stream_base& m_stream;
    std::ifstream m_fileStream;
    data m_data;
    int m_currentLine{ 0 };
    std::optional<error> m_error;
    std::vector<std::string> m_layerNames;
};

}   // namespace odxf
