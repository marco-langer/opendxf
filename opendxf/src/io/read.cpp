// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/read.hpp"

#include "opendxf/io/read_stream_base.hpp"
#include "reader.hpp"

#include <utility>

namespace {

class document_stream final : public odxf::read_stream_base
{
public:
    odxf::document document() && { return std::move(m_document); }

private:
    void header(const odxf::header& header) override { m_document.header = header; }
    void lineType(const odxf::line_type& lineType) override
    {
        m_document.tables.lineTypes.push_back(lineType);
    }
    void layer(const odxf::layer& layer) override { m_document.tables.layers.push_back(layer); }

    void arc(const odxf::arc& arc) override { m_document.entities.push_back(arc); }
    void circle(const odxf::circle& circle) override { m_document.entities.push_back(circle); }
    void line(const odxf::line& line) override { m_document.entities.push_back(line); }
    void lwPolyline(const odxf::lw_polyline& lwPolyline) override
    {
        m_document.entities.push_back(lwPolyline);
    }

    odxf::document m_document;
};

}   // namespace

namespace odxf {

std::expected<document, error> read(const std::filesystem::path& filePath)
{
    document_stream stream;

    return read(filePath, stream).transform([&stream] { return std::move(stream).document(); });
}

std::expected<void, error> read(const std::filesystem::path& filePath, read_stream_base& stream)
{
    reader reader{ stream };

    return reader.readAll(filePath);
}

}   // namespace odxf
