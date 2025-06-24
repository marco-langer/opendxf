#include "reader.hpp"

#include "opendxf/ireadstream.hpp"

namespace odxf {

Reader::Reader(IReadStream& stream)
    : m_stream{ stream }
{
}

}   // namespace odxf
