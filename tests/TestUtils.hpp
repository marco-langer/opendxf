// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/document.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/io/ireadstream.hpp"
#include "opendxf/layer.hpp"
#include "opendxf/tables.hpp"

class ReadStream final : public odxf::IReadStream
{
public:
    const odxf::Document& document() const& { return m_document; }

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

odxf::Document createExampleDocument();
