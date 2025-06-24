#include "opendxf/ireadstream.hpp"

namespace odxf {

IReadStream::~IReadStream() = default;

void IReadStream::header(const Header& /* header */) {}

}   // namespace odxf
