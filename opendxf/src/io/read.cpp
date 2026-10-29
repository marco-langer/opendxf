// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/read.hpp"

#include "opendxf/io/read_stream_base.hpp"
#include "reader.hpp"

#include <utility>

namespace {

class DocumentStream final : public odxf::IReadStream
{
public:
    odxf::Document document() && { return std::move(m_document); }

private:
    void header(const odxf::Header& header) override { m_document.header = header; }
    void lineType(const odxf::LineType& lineType) override
    {
        m_document.tables.lineTypes.push_back(lineType);
    }
    void layer(const odxf::Layer& layer) override { m_document.tables.layers.push_back(layer); }

    void arc(const odxf::Arc& arc) override { m_document.entities.push_back(arc); }
    void circle(const odxf::Circle& circle) override { m_document.entities.push_back(circle); }
    void line(const odxf::Line& line) override { m_document.entities.push_back(line); }
    void lwPolyline(const odxf::LWPolyline& lwPolyline) override
    {
        m_document.entities.push_back(lwPolyline);
    }

    odxf::Document m_document;
};

}   // namespace

namespace odxf {

std::expected<Document, Error> read(const std::filesystem::path& filePath)
{
    DocumentStream stream;

    return read(filePath, stream).transform([&stream] { return std::move(stream).document(); });
}

std::expected<void, Error> read(const std::filesystem::path& filePath, IReadStream& stream)
{
    Reader reader{ stream };

    return reader.readAll(filePath);
}

}   // namespace odxf
