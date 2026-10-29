// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/ireadstream.hpp"

#include "opendxf/entities.hpp"
#include "opendxf/tables.hpp"

namespace odxf {

IReadStream::~IReadStream() = default;

void IReadStream::header(const Header& /* header */) {}

void IReadStream::lineType(const LineType& /* lineType */) {}

void IReadStream::layer(const Layer& /* layer */) {}

void IReadStream::arc(const Arc& /* arc */) {}

void IReadStream::circle(const Circle& /* circle */) {}

void IReadStream::line(const Line& /* line */) {}

void IReadStream::lwPolyline(const LWPolyline& /* lwPolyline */) {}

}   // namespace odxf
