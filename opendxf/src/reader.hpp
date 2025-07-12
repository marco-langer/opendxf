#pragma once

#include "opendxf/entities.hpp"
#include "opendxf/error.hpp"
#include "opendxf/header.hpp"

#include <tl/expected.hpp>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
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

    tl::expected<void, Error> readAll(const std::filesystem::path& filePath);

private:
    bool open(const std::filesystem::path& filePath);

    tl::expected<void, Error> readHeader();
    tl::expected<HeaderEntry, Error> readHeaderEntry();

    tl::expected<void, Error> readTables();
    tl::expected<void, Error> readLayer();

    tl::expected<void, Error> readBlocks();

    tl::expected<void, Error> readEntities();
    tl::expected<void, Error> readLine();
    tl::expected<void, Error> readCircle();
    tl::expected<void, Error> readArc();

    bool readNext();

    bool hasError() const;
    tl::expected<void, Error> makeError() const;
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
