// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/read.hpp"

#include "opendxf/io/read_stream_base.hpp"
#include "reader.hpp"

#include <utility>

namespace {

class drawing_stream final : public odxf::read_stream_base
{
public:
    odxf::drawing drawing() && { return std::move(m_drawing); }

private:
    void header(const odxf::header& header) override { m_drawing.header = header; }
    void line_type(const odxf::line_type& line_type) override
    {
        m_drawing.tables.line_types.push_back(line_type);
    }
    void layer(const odxf::layer& layer) override { m_drawing.tables.layers.push_back(layer); }

    void arc(const odxf::arc& arc) override { m_drawing.entities.push_back(arc); }
    void circle(const odxf::circle& circle) override { m_drawing.entities.push_back(circle); }
    void ellipse(const odxf::ellipse& ellipse) override { m_drawing.entities.push_back(ellipse); }
    void line(const odxf::line& line) override { m_drawing.entities.push_back(line); }
    void lw_polyline(const odxf::lw_polyline& lw_polyline) override
    {
        m_drawing.entities.push_back(lw_polyline);
    }
    void point(const odxf::point& point) override { m_drawing.entities.push_back(point); }
    void ray(const odxf::ray& ray) override { m_drawing.entities.push_back(ray); }

    odxf::drawing m_drawing;
};

}   // namespace

namespace odxf {

std::expected<drawing, error> read(const std::filesystem::path& file_path)
{
    drawing_stream stream;

    return read(file_path, stream).transform([&stream] { return std::move(stream).drawing(); });
}

std::expected<void, error> read(const std::filesystem::path& file_path, read_stream_base& stream)
{
    reader reader{ stream };

    return reader.read_all(file_path);
}

}   // namespace odxf
