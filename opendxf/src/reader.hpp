#pragma once

#include "opendxf/read.hpp"

namespace odxf {

class IReadStream;

class Reader
{
public:
    Reader(IReadStream& stream);

    Reader(const Reader&) = delete;
    Reader(Reader&&) = delete;
    Reader& operator=(const Reader&) = delete;
    Reader& operator=(Reader&&) = delete;

private:
    IReadStream& m_stream;
};

}   // namespace odxf
