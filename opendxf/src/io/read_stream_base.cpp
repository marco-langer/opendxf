// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/read_stream_base.hpp"

#include "opendxf/entities.hpp"
#include "opendxf/tables.hpp"

namespace odxf {

read_stream_base::~read_stream_base() = default;

void read_stream_base::header(const odxf::header& /* header */) {}

void read_stream_base::line_type(const odxf::line_type& /* line_type */) {}

void read_stream_base::layer(const odxf::layer& /* layer */) {}

void read_stream_base::arc(const odxf::arc& /* arc */) {}

void read_stream_base::circle(const odxf::circle& /* circle */) {}

void read_stream_base::line(const odxf::line& /* line */) {}

void read_stream_base::lw_polyline(const odxf::lw_polyline& /* lw_polyline */) {}

}   // namespace odxf
