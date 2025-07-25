// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "reader.hpp"

#include "opendxf/ireadstream.hpp"
#include "opendxf/layer.hpp"

#include <fmt/format.h>

#include <charconv>

namespace {

template <typename T>
std::optional<T> parseAs(const std::string& value)
{
    const char* first{ value.c_str() };
    if (!value.empty()) {
        while (*first == ' ') {
            ++first;
        }
    }

    T result;
    const auto [_, errorCode]{ std::from_chars(first, first + value.size(), result) };

    return errorCode == std::errc() ? result : std::optional<T>{};
}

}   // namespace

namespace odxf {

Reader::Reader(IReadStream& stream)
    : m_stream{ stream }
{
}

tl::expected<void, Error> Reader::readAll(const std::filesystem::path& filePath)
{
    if (!open(filePath)) {
        return makeError();
    }

    if (tl::expected<void, Error> maybeError = readHeader(); !maybeError) {
        return maybeError;
    }

    if (tl::expected<void, Error> maybeError = readTables(); !maybeError) {
        return maybeError;
    }

    if (tl::expected<void, Error> maybeError = readBlocks(); !maybeError) {
        return maybeError;
    }

    if (tl::expected<void, Error> maybeError = readEntities(); !maybeError) {
        return maybeError;
    }

    if (!readNext()) {
        return makeError();
    }

    if (isEOF()) {
        return {};
    }

    return tl::make_unexpected(Error{ .lineNumber = m_currentLine, .what = "EOF missing" });
}

bool Reader::open(const std::filesystem::path& filePath)
{
    m_fileStream.open(filePath);

    if (!m_fileStream.is_open()) {
        m_error = Error{
            .type = Error::Type::FileOpenError,
            .what = fmt::format(
                "unable to open file {}",
                reinterpret_cast<const char*>(filePath.u8string().c_str())),
        };

        return false;
    }

    return true;
}

tl::expected<void, Error> Reader::readHeader()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected section begin",
        });
    }

    if (!readNext()) {
        return makeError();
    }

    if (!isHeaderBegin()) {
        return {};   // header is optional
    }

    if (!readNext()) {
        return makeError();
    }

    Header header;
    while (!isSectionEnd()) {
        if (hasError()) {
            return tl::make_unexpected(m_error.value());
        }

        tl::expected<HeaderEntry, Error> maybeHeaderEntry{ readHeaderEntry() };
        if (!maybeHeaderEntry) {
            return tl::make_unexpected(maybeHeaderEntry.error());
        }

        HeaderEntry& headerEntry{ *maybeHeaderEntry };
        header.entries.emplace(headerEntry);
    }

    m_stream.header(header);

    return {};
}

tl::expected<HeaderEntry, Error> Reader::readHeaderEntry()
{
    if (m_data.groupCode != 9) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected group code 9 for header variable name",
        });
    }

    HeaderKey key{ m_data.value };

    if (!readNext()) {
        return tl::make_unexpected(m_error.value());
    }

    switch (m_data.groupCode) {
    case 1:
    case 2:
    case 3: {
        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    };

    case 5: {
        if (key != "$HANDSEED") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 6: {
        if (key != "$CELTYPE" && key != "$DIMLTYPE" && key != "$DIMLTEX1" && key != "$DIMLTEX2") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 7: {
        if (key != "$TEXTSTYLE" && key != "$DIMTXSTY") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 8: {
        if (key != "$CLAYER") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 10: {
        tl::expected<std::variant<Coordinate2d, Coordinate3d>, Error> maybeCoordinate{
            readHeaderCoordinate()
        };

        if (!maybeCoordinate) {
            return tl::make_unexpected(maybeCoordinate.error());
        }

        HeaderValue value{ std::visit(
            [](const auto& coordinate) -> HeaderValue { return coordinate; },
            maybeCoordinate.value()) };

        return std::pair{ std::move(key), value };
    }

    case 40: {
        const std::optional<double> maybeValue{ parseAs<double>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 50: {
        if (key != "$ANGBASE") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }
        const std::optional<double> maybeValue{ parseAs<double>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 62: {
        if (key != "$CECOLOR" && key != "$INTERFERECOLOR") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 70: {
        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 280: {
        if (key != "$ENDCAPS" && key != "$JOINSTYLE" && key != "$SORTENTS" && key != "$INDEXCTL"
            && key != "$HIDETEXT" && key != "$HALOGAP" && key != "$OBSLTYPE"
            && key != "$INTERSECTIONDISPLAY" && key != "$DIMASSOC" && key != "$LOFTNORMALS"
            && key != "$LIGHTGLYPHDISPLAY" && key != "$TILEMODELIGHTSYNCH" && key != "$SOLIDHIST"
            && key != "$SHOWHIST" && key != "$DWFFRAME" && key != "$DGNFRAME"
            && key != "$CSHADOW") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 290: {
        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue != 0 };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 370: {
        if (key != "$CELWEIGHT") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 380: {
        if (key != "$CEPSNTYPE") {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    default: {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine - 1,
            .what =
                fmt::format("unexpected group code {} for header variable name", m_data.groupCode),
        });
    }
    }

    return {};
}

tl::expected<std::variant<Coordinate2d, Coordinate3d>, Error> Reader::readHeaderCoordinate()
{
    double x{ 0.0 };
    double y{ 0.0 };
    std::optional<double> z;

    while (m_data.groupCode == 10 || m_data.groupCode == 20 || m_data.groupCode == 30) {
        switch (m_data.groupCode) {
        case 10: {
            const std::optional<double> maybeX{ parseAs<double>(m_data.value) };
            if (!maybeX) {
                return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
            }
            x = *maybeX;

            break;
        }

        case 20: {
            const std::optional<double> maybeY{ parseAs<double>(m_data.value) };
            if (!maybeY) {
                return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
            }
            y = *maybeY;

            break;
        }

        case 30: {
            const std::optional<double> maybeZ{ parseAs<double>(m_data.value) };
            if (!maybeZ) {
                return tl::make_unexpected(Error{ .lineNumber = m_currentLine });
            }
            z = *maybeZ;

            break;
        }
        }

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }
    }

    if (z.has_value()) {
        return Coordinate3d{ x, y, *z };
    }

    return Coordinate2d{ x, y };
}

tl::expected<void, Error> Reader::readTables()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected section begin",
        });
    }

    if (!readNext()) {
        return makeError();
    }

    if (!isTablesBegin()) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected tables begin",
        });
    }

    while (!isSectionEnd()) {
        if (hasError()) {
            return makeError();
        }

        if (isLayer()) {
            tl::expected<void, Error> maybeResult{ readLayer() };
            if (!maybeResult) {
                return maybeResult;
            }
        } else {
            readNext();
        }

        if (hasError()) {
            return makeError();
        }
    }

    return {};
}

tl::expected<void, Error> Reader::readLayer()
{
    if (!readNext()) {
        return tl::make_unexpected(m_error.value());
    }

    Layer layer;
    while (m_data.groupCode != 0) {
        switch (m_data.groupCode) {
        case 2: {
            layer.name = m_data.value;

            break;
        }

        case 70: {
            const std::optional<int> maybeFlags{ parseAs<int>(m_data.value) };
            if (maybeFlags.has_value()) {
                layer.flags = static_cast<odxf::Layer::Flags>(*maybeFlags);
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }
        }

        if (!readNext()) {
            return makeError();
        }
    }

    m_stream.layer(layer);

    return tl::expected<void, Error>();
}

tl::expected<void, Error> Reader::readBlocks()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected section begin",
        });
    }

    if (!readNext()) {
        return makeError();
    }

    if (!isBlocksBegin()) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected blocks begin",
        });
    }

    while (readNext() && !isSectionEnd()) {
        if (hasError()) {
            return makeError();
        }
    }

    return {};
}

tl::expected<void, Error> Reader::readEntities()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected section begin",
        });
    }

    if (!readNext()) {
        return makeError();
    }

    if (!isEntitiesBegin()) {
        return tl::make_unexpected(Error{
            .lineNumber = m_currentLine,
            .what = "expected entities begin",
        });
    }

    while (!isSectionEnd()) {
        if (hasError()) {
            return makeError();
        }

        if (isLine()) {
            tl::expected<void, Error> maybeResult{ readLine() };
            if (!maybeResult) {
                return maybeResult;
            }

        } else if (isCircle()) {
            tl::expected<void, Error> maybeResult{ readCircle() };
            if (!maybeResult) {
                return maybeResult;
            }
        } else if (isArc()) {
            tl::expected<void, Error> maybeResult{ readArc() };
            if (!maybeResult) {
                return maybeResult;
            }
        } else if (isLWPolyline()) {
            tl::expected<void, Error> maybeResult{ readLWPolyline() };
            if (!maybeResult) {
                return maybeResult;
            }
        } else {
            readNext();
        }

        if (hasError()) {
            return makeError();
        }
    }

    return {};
}

tl::expected<void, Error> Reader::readLine()
{
    if (!readNext()) {
        return tl::make_unexpected(m_error.value());
    }

    Line line;

    while (m_data.groupCode != 0) {
        switch (m_data.groupCode) {
        case 8: {
            line.layer = m_data.value;

            break;
        }

        case 10: {
            const std::optional<double> maybeX{ parseAs<double>(m_data.value) };
            if (maybeX.has_value()) {
                line.start.x = *maybeX;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        };

        case 20: {
            const std::optional<double> maybeY{ parseAs<double>(m_data.value) };
            if (maybeY.has_value()) {
                line.start.y = *maybeY;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 11: {
            const std::optional<double> maybeX{ parseAs<double>(m_data.value) };
            if (maybeX.has_value()) {
                line.end.x = *maybeX;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 21: {
            const std::optional<double> maybeY{ parseAs<double>(m_data.value) };
            if (maybeY.has_value()) {
                line.end.y = *maybeY;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }
        }

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }
    }

    m_stream.line(line);

    return {};
}

tl::expected<void, Error> Reader::readCircle()
{
    if (!readNext()) {
        return tl::make_unexpected(m_error.value());
    }

    Circle circle;

    while (m_data.groupCode != 0) {
        switch (m_data.groupCode) {
        case 8: {
            circle.layer = m_data.value;

            break;
        }

        case 10: {
            const std::optional<double> maybeX{ parseAs<double>(m_data.value) };
            if (maybeX.has_value()) {
                circle.center.x = *maybeX;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        };

        case 20: {
            const std::optional<double> maybeY{ parseAs<double>(m_data.value) };
            if (maybeY.has_value()) {
                circle.center.y = *maybeY;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 40: {
            const std::optional<double> maybeRadius{ parseAs<double>(m_data.value) };
            if (maybeRadius.has_value()) {
                circle.radius = *maybeRadius;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }
        }

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }
    }

    m_stream.circle(circle);

    return tl::expected<void, Error>();
}

tl::expected<void, Error> Reader::readArc()
{
    if (!readNext()) {
        return tl::make_unexpected(m_error.value());
    }

    Arc arc;

    while (m_data.groupCode != 0) {
        switch (m_data.groupCode) {
        case 8: {
            arc.layer = m_data.value;

            break;
        }

        case 10: {
            const std::optional<double> maybeX{ parseAs<double>(m_data.value) };
            if (maybeX.has_value()) {
                arc.center.x = *maybeX;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        };

        case 20: {
            const std::optional<double> maybeY{ parseAs<double>(m_data.value) };
            if (maybeY.has_value()) {
                arc.center.y = *maybeY;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 40: {
            const std::optional<double> maybeRadius{ parseAs<double>(m_data.value) };
            if (maybeRadius.has_value()) {
                arc.radius = *maybeRadius;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 50: {
            const std::optional<double> maybeStartAngle{ parseAs<double>(m_data.value) };
            if (maybeStartAngle.has_value()) {
                arc.startAngle = *maybeStartAngle;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 51: {
            const std::optional<double> maybeEndAngle{ parseAs<double>(m_data.value) };
            if (maybeEndAngle.has_value()) {
                arc.endAngle = *maybeEndAngle;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }
        }

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }
    }

    m_stream.arc(arc);

    return tl::expected<void, Error>();
}

tl::expected<void, Error> Reader::readLWPolyline()
{
    if (!readNext()) {
        return tl::make_unexpected(m_error.value());
    }

    LWPolyline lwPolyline;

    Vertex vertex;
    int numXY{ 0 };

    while (m_data.groupCode != 0) {
        switch (m_data.groupCode) {
        case 90: {
            const std::optional<int> maybeVerticesCount{ parseAs<int>(m_data.value) };
            if (maybeVerticesCount.has_value()) {
                if (*maybeVerticesCount >= 0) {
                    lwPolyline.vertices.reserve(static_cast<std::size_t>(*maybeVerticesCount));
                }
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 70: {
            const std::optional<int> maybeFlag{ parseAs<int>(m_data.value) };
            if (maybeFlag.has_value()) {
                lwPolyline.isClosed = (*maybeFlag & 1 != 0);
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 10: {
            const std::optional<double> maybeX{ parseAs<double>(m_data.value) };
            if (numXY == 2) {
                lwPolyline.vertices.push_back(vertex);
                numXY = 0;
                vertex.bulge.reset();
            }
            if (maybeX.has_value()) {
                vertex.position.x = *maybeX;
                numXY++;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 20: {
            const std::optional<double> maybeY{ parseAs<double>(m_data.value) };
            if (numXY == 2) {
                lwPolyline.vertices.push_back(vertex);
                numXY = 0;
                vertex.bulge.reset();
            }
            if (maybeY.has_value()) {
                vertex.position.y = *maybeY;
                numXY++;
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }

        case 42: {
            const std::optional<double> maybeBulge{ parseAs<double>(m_data.value) };
            if (maybeBulge.has_value()) {
                vertex.bulge = *maybeBulge;

                if (numXY == 2) {
                    lwPolyline.vertices.push_back(vertex);
                    numXY = 0;
                    vertex.bulge.reset();
                }
            } else {
                return tl::make_unexpected(Error{
                    .lineNumber = m_currentLine,
                });
            }

            break;
        }
        }

        if (!readNext()) {
            return tl::make_unexpected(m_error.value());
        }
    }

    if (numXY == 2) {
        lwPolyline.vertices.push_back(vertex);
    }

    m_stream.lwPolyline(lwPolyline);

    return tl::expected<void, Error>();
}

bool Reader::readNext()
{
    bool success{ false };
    do {
        success = readNextSingle();
    } while (m_data.groupCode == 999 && success);

    return success;
}

bool Reader::readNextSingle()
{
    m_currentLine++;

    if (!std::getline(m_fileStream, m_data.value)) {
        m_error = Error{
            .lineNumber = m_currentLine,
            .what = "unable to read line",
        };

        return false;
    }

    const char* first{ m_data.value.c_str() };
    if (!m_data.value.empty()) {
        while (*first == ' ') {
            ++first;
        }
    }
    const auto [_, errorCode]{ std::from_chars(
        first, first + m_data.value.size(), m_data.groupCode) };
    if (errorCode != std::errc()) {
        m_error = Error{
            .lineNumber = m_currentLine,
            .what = "unable to parse group code",
        };

        return false;
    }

    m_currentLine++;
    if (!std::getline(m_fileStream, m_data.value)) {
        m_error = Error{
            .lineNumber = m_currentLine,
            .what = "unable to read line",
        };

        return false;
    }

    return true;
}

bool Reader::hasError() const { return m_error.has_value(); }

tl::expected<void, Error> Reader::makeError() const { return tl::make_unexpected(m_error.value()); }

bool Reader::isSectionBegin() const { return m_data.groupCode == 0 && m_data.value == "SECTION"; }

bool Reader::isSectionEnd() const { return m_data.groupCode == 0 && m_data.value == "ENDSEC"; }

bool Reader::isHeaderBegin() const { return m_data.groupCode == 2 && m_data.value == "HEADER"; }

bool Reader::isTablesBegin() const { return m_data.groupCode == 2 && m_data.value == "TABLES"; }

bool Reader::isBlocksBegin() const { return m_data.groupCode == 2 && m_data.value == "BLOCKS"; }

bool Reader::isEntitiesBegin() const { return m_data.groupCode == 2 && m_data.value == "ENTITIES"; }

bool Reader::isTableBegin() const { return m_data.groupCode == 0 && m_data.value == "TABLE"; }

bool Reader::isTableEnd() const { return m_data.groupCode == 0 && m_data.value == "ENDTAB"; }

bool Reader::isLayerBegin() const { return m_data.groupCode == 2 && m_data.value == "LAYER"; }

bool Reader::isLayer() const { return m_data.groupCode == 0 && m_data.value == "LAYER"; }

bool Reader::isLine() const { return m_data.groupCode == 0 && m_data.value == "LINE"; }

bool odxf::Reader::isCircle() const { return m_data.groupCode == 0 && m_data.value == "CIRCLE"; }

bool Reader::isArc() const { return m_data.groupCode == 0 && m_data.value == "ARC"; }

bool Reader::isLWPolyline() const { return m_data.groupCode == 0 && m_data.value == "LWPOLYLINE"; }

bool Reader::isEOF() const { return m_data.groupCode == 0 && m_data.value == "EOF"; }

}   // namespace odxf
