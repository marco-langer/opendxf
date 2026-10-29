// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

namespace odxf {

struct arc;
struct circle;
struct header;
struct layer;
struct line;
struct line_type;
struct lw_polyline;

class read_stream_base
{
public:
    read_stream_base() = default;
    virtual ~read_stream_base() = 0;

    virtual void header(const odxf::header& header);
    virtual void lineType(const line_type& lineType);
    virtual void layer(const odxf::layer& layer);

    virtual void arc(const odxf::arc& arc);
    virtual void circle(const odxf::circle& circle);
    virtual void line(const odxf::line& line);
    virtual void lwPolyline(const lw_polyline& lwPolyline);

protected:
    read_stream_base(const read_stream_base&) = default;
    read_stream_base(read_stream_base&&) = default;
    read_stream_base& operator=(const read_stream_base&) = default;
    read_stream_base& operator=(read_stream_base&&) = default;
};

}   // namespace odxf
