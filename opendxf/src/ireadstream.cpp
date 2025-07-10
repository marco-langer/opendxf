#include "opendxf/ireadstream.hpp"

#include "opendxf/entities.hpp"

namespace odxf {

IReadStream::~IReadStream() = default;

void IReadStream::header(const Header& /* header */) {}

void IReadStream::layer(const Layer& /* layer */) {}

void IReadStream::line(const Line& /* line */) {}

void IReadStream::circle(const Circle& /* circle */) {}

}   // namespace odxf
