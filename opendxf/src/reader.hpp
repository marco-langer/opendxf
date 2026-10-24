// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/error.hpp"
#include "opendxf/header.hpp"

#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace odxf {

class IReadStream;

class Reader
{
public:
    Reader(IReadStream& stream);

    Reader(const Reader&) = delete;
    Reader(Reader&&) = delete;
    Reader& operator=(const Reader&) = delete;
    Reader& operator=(Reader&&) = delete;

    std::expected<void, Error> readAll(const std::filesystem::path& filePath);

private:
    bool open(const std::filesystem::path& filePath);

    std::expected<void, Error> readHeader();
    std::expected<HeaderEntry, Error> readHeaderEntry();
    std::expected<std::variant<Coordinate2d, Coordinate3d>, Error> readHeaderCoordinate();

    std::expected<void, Error> readTables();
    std::expected<void, Error> readLayer();

    std::expected<void, Error> readBlocks();

    std::expected<void, Error> readEntities();
    std::expected<void, Error> readLine();
    std::expected<void, Error> readCircle();
    std::expected<void, Error> readArc();
    std::expected<void, Error> readLWPolyline();

    bool readNext();
    bool readNextSingle();

    bool hasError() const;
    std::expected<void, Error> makeError() const;
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

    bool isLine() const;
    bool isCircle() const;
    bool isArc() const;
    bool isLWPolyline() const;

    bool isEOF() const;

    struct Data
    {
        int groupCode;
        std::string value;
    };

    IReadStream& m_stream;
    std::ifstream m_fileStream;
    Data m_data;
    int m_currentLine{ 0 };
    std::optional<Error> m_error;
    std::vector<std::string> m_layerNames;
};

}   // namespace odxf
