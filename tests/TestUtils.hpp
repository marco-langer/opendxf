// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/document.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/ireadstream.hpp"
#include "opendxf/layer.hpp"

class ReadStream final : public odxf::IReadStream
{
public:
    const odxf::Document& document() const& { return m_document; }

private:
    void header(const odxf::Header& header) override { m_document.header = header; }
    void layer(const odxf::Layer& layer) override { m_document.tables.layers.push_back(layer); }

    void arc(const odxf::Arc& arc) override { m_document.entities.arcs.push_back(arc); }
    void circle(const odxf::Circle& circle) override
    {
        m_document.entities.circles.push_back(circle);
    }
    void line(const odxf::Line& line) override { m_document.entities.lines.push_back(line); }
    void lwPolyline(const odxf::LWPolyline& lwPolyline) override
    {
        m_document.entities.lwPolylines.push_back(lwPolyline);
    }

    odxf::Document m_document;
};

odxf::Document createExampleDocument();
