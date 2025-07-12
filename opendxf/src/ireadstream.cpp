#include "opendxf/ireadstream.hpp"

#include "opendxf/entities.hpp"

namespace odxf {

IReadStream::~IReadStream() = default;

void IReadStream::header(const Header& /* header */) {}

void IReadStream::layer(const Layer& /* layer */) {}

void IReadStream::arc(const Arc& /* arc */) {}

void IReadStream::circle(const Circle& /* circle */) {}

void IReadStream::line(const Line& /* line */) {}

void IReadStream::lwPolyline(const LWPolyline& /* lwPolyline */) {}

}   // namespace odxf
