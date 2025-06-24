#pragma once

namespace odxf {

class Header;

class IReadStream
{
public:
    IReadStream() = default;
    virtual ~IReadStream() = 0;

    virtual void header(const Header& header);

protected:
    IReadStream(const IReadStream&) = default;
    IReadStream(IReadStream&&) = default;
    IReadStream& operator=(const IReadStream&) = default;
    IReadStream& operator=(IReadStream&&) = default;
};

}   // namespace odxf
