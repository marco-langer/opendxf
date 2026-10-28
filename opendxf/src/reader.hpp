// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/error.hpp"
#include "opendxf/header.hpp"

#include <concepts>
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
    std::expected<void, Error> readLineType();
    std::expected<void, Error> readLayer();

    std::expected<void, Error> readBlocks();

    std::expected<void, Error> readEntities();
    std::expected<void, Error> readLine();
    std::expected<void, Error> readCircle();
    std::expected<void, Error> readArc();
    std::expected<void, Error> readLWPolyline();

    // Reads the group codes of one entity up to the next group code 0. readGroupCode is called for
    // each group code and returns false on an error.
    template <std::derived_from<Entity> E, std::predicate<E&> F>
    std::expected<E, Error> readEntity(F readGroupCode);
    // Reads the group codes all entities share, skips unknown ones. Returns false on an error.
    bool readEntityProperty(Entity& entity);

    bool readNext();
    bool readNextSingle();

    // parse the current value into target, on failure sets the error and returns false
    template <typename T>
    bool parseInto(T& target);
    template <typename T>
    bool parseInto(std::optional<T>& target);

    bool hasError() const;
    std::expected<void, Error> makeError() const;
    // an Error at the current line, without a description
    std::unexpected<Error> parseError() const;
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
