// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

namespace odxf {

class Arc;
class Circle;
class Header;
class Layer;
class Line;
class LWPolyline;

class IReadStream
{
public:
    IReadStream() = default;
    virtual ~IReadStream() = 0;

    virtual void header(const Header& header);
    virtual void layer(const Layer& layer);

    virtual void arc(const Arc& arc);
    virtual void circle(const Circle& circle);
    virtual void line(const Line& line);
    virtual void lwPolyline(const LWPolyline& lwPolyline);

protected:
    IReadStream(const IReadStream&) = default;
    IReadStream(IReadStream&&) = default;
    IReadStream& operator=(const IReadStream&) = default;
    IReadStream& operator=(IReadStream&&) = default;
};

}   // namespace odxf
