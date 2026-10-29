// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "reader.hpp"
#include "transparency.hpp"

#include "opendxf/io/read_stream_base.hpp"
#include "opendxf/layer.hpp"
#include "opendxf/tables.hpp"

#include <charconv>
#include <format>

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
    const auto [_, errorCode]{ std::from_chars(first, value.c_str() + value.size(), result) };

    return errorCode == std::errc() ? result : std::optional<T>{};
}

}   // namespace

namespace odxf {

template <typename T>
bool Reader::parseInto(T& target)
{
    const std::optional<T> maybeValue{ parseAs<T>(m_data.value) };
    if (!maybeValue) {
        m_error = parseError().error();
        return false;
    }

    target = *maybeValue;
    return true;
}

template <typename T>
bool Reader::parseInto(std::optional<T>& target)
{
    T value;
    if (!parseInto(value)) {
        return false;
    }

    target = value;
    return true;
}

Reader::Reader(IReadStream& stream)
    : m_stream{ stream }
{
}

std::expected<void, Error> Reader::readAll(const std::filesystem::path& filePath)
{
    if (!open(filePath)) {
        return makeError();
    }

    if (std::expected<void, Error> maybeError = readHeader(); !maybeError) {
        return maybeError;
    }

    if (std::expected<void, Error> maybeError = readTables(); !maybeError) {
        return maybeError;
    }

    if (std::expected<void, Error> maybeError = readBlocks(); !maybeError) {
        return maybeError;
    }

    if (std::expected<void, Error> maybeError = readEntities(); !maybeError) {
        return maybeError;
    }

    if (!readNext()) {
        return makeError();
    }

    if (isEOF()) {
        return {};
    }

    return std::unexpected(Error{ .lineNumber = m_currentLine, .what = "EOF missing" });
}

bool Reader::open(const std::filesystem::path& filePath)
{
    m_fileStream.open(filePath);

    if (!m_fileStream.is_open()) {
        m_error = Error{
            .type = Error::Type::FileOpenError,
            .what = std::format(
                "unable to open file {}",
                reinterpret_cast<const char*>(filePath.u8string().c_str())),
        };

        return false;
    }

    return true;
}

std::expected<void, Error> Reader::readHeader()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return std::unexpected(
            Error{
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
            return std::unexpected(m_error.value());
        }

        std::expected<HeaderEntry, Error> maybeHeaderEntry{ readHeaderEntry() };
        if (!maybeHeaderEntry) {
            return std::unexpected(maybeHeaderEntry.error());
        }

        HeaderEntry& headerEntry{ *maybeHeaderEntry };
        header.entries.emplace(headerEntry);
    }

    m_stream.header(header);

    return {};
}

std::expected<HeaderEntry, Error> Reader::readHeaderEntry()
{
    if (m_data.groupCode != 9) {
        return std::unexpected(
            Error{
                .lineNumber = m_currentLine,
                .what = "expected group code 9 for header variable name",
            });
    }

    HeaderKey key{ m_data.value };

    if (!readNext()) {
        return std::unexpected(m_error.value());
    }

    switch (m_data.groupCode) {
    case 1:
    case 2:
    case 3: {
        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    };

    case 5: {
        if (key != "$HANDSEED") {
            return parseError();
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 6: {
        if (key != "$CELTYPE" && key != "$DIMLTYPE" && key != "$DIMLTEX1" && key != "$DIMLTEX2") {
            return parseError();
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 7: {
        if (key != "$TEXTSTYLE" && key != "$DIMTXSTY") {
            return parseError();
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 8: {
        if (key != "$CLAYER") {
            return parseError();
        }

        HeaderValue value{ m_data.value };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 10: {
        std::expected<std::variant<Coordinate2d, Coordinate3d>, Error> maybeCoordinate{
            readHeaderCoordinate()
        };

        if (!maybeCoordinate) {
            return std::unexpected(maybeCoordinate.error());
        }

        HeaderValue value{ std::visit(
            [](const auto& coordinate) -> HeaderValue { return coordinate; },
            maybeCoordinate.value()) };

        return std::pair{ std::move(key), value };
    }

    case 40: {
        const std::optional<double> maybeValue{ parseAs<double>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 50: {
        if (key != "$ANGBASE") {
            return parseError();
        }
        const std::optional<double> maybeValue{ parseAs<double>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 62: {
        if (key != "$CECOLOR" && key != "$INTERFERECOLOR") {
            return parseError();
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 70: {
        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return std::unexpected(m_error.value());
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
            return parseError();
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 290: {
        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue != 0 };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 370: {
        if (key != "$CELWEIGHT") {
            return parseError();
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 380: {
        if (key != "$CEPSNTYPE") {
            return parseError();
        }

        const std::optional<int> maybeValue{ parseAs<int>(m_data.value) };
        if (!maybeValue) {
            return parseError();
        }

        HeaderValue value{ *maybeValue };

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    default: {
        return std::unexpected(
            Error{
                .lineNumber = m_currentLine - 1,
                .what = std::format(
                    "unexpected group code {} for header variable name", m_data.groupCode),
            });
    }
    }

    return {};
}

std::expected<std::variant<Coordinate2d, Coordinate3d>, Error> Reader::readHeaderCoordinate()
{
    double x{ 0.0 };
    double y{ 0.0 };
    std::optional<double> z;

    while (m_data.groupCode == 10 || m_data.groupCode == 20 || m_data.groupCode == 30) {
        switch (m_data.groupCode) {
        case 10: {
            if (!parseInto(x)) {
                return std::unexpected(m_error.value());
            }

            break;
        }

        case 20: {
            if (!parseInto(y)) {
                return std::unexpected(m_error.value());
            }

            break;
        }

        case 30: {
            if (!parseInto(z)) {
                return std::unexpected(m_error.value());
            }

            break;
        }
        }

        if (!readNext()) {
            return std::unexpected(m_error.value());
        }
    }

    if (z.has_value()) {
        return Coordinate3d{ x, y, *z };
    }

    return Coordinate2d{ x, y };
}

std::expected<void, Error> Reader::readTables()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return std::unexpected(
            Error{
                .lineNumber = m_currentLine,
                .what = "expected section begin",
            });
    }

    if (!readNext()) {
        return makeError();
    }

    if (!isTablesBegin()) {
        return std::unexpected(
            Error{
                .lineNumber = m_currentLine,
                .what = "expected tables begin",
            });
    }

    while (!isSectionEnd()) {
        if (hasError()) {
            return makeError();
        }

        if (isLayer()) {
            std::expected<void, Error> maybeResult{ readLayer() };
            if (!maybeResult) {
                return maybeResult;
            }
        } else if (isLineType()) {
            std::expected<void, Error> maybeResult{ readLineType() };
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

std::expected<void, Error> Reader::readLineType()
{
    if (!readNext()) {
        return std::unexpected(m_error.value());
    }

    LineType lineType;
    while (m_data.groupCode != 0) {
        switch (m_data.groupCode) {
        case 2: {
            lineType.name = m_data.value;

            break;
        }

        case 3: {
            lineType.displayName = m_data.value;

            break;
        }

        case 70: {
            if (!parseInto(lineType.flags)) {
                return makeError();
            }

            break;
        }

        case 49: {
            const std::optional<double> maybeLength{ parseAs<double>(m_data.value) };
            if (maybeLength.has_value()) {
                lineType.elements.push_back(LineTypeElement{ .length = *maybeLength });
            } else {
                return parseError();
            }

            break;
        }

        // Follows the 49 of its element.
        case 74: {
            const std::optional<int> maybeFlags{ parseAs<int>(m_data.value) };
            if (!maybeFlags.has_value() || lineType.elements.empty()) {
                return parseError();
            }
            lineType.elements.back().complexFlags = *maybeFlags;

            break;
        }
        }

        if (!readNext()) {
            return makeError();
        }
    }

    m_stream.lineType(lineType);

    return std::expected<void, Error>();
}

std::expected<void, Error> Reader::readLayer()
{
    if (!readNext()) {
        return std::unexpected(m_error.value());
    }

    Layer layer;
    // The application that owns the xdata read next, empty before the first group code 1001.
    std::string xdataApp;
    while (m_data.groupCode != 0) {
        switch (m_data.groupCode) {
        case 2: {
            layer.name = m_data.value;

            break;
        }

        case 62: {
            if (!parseInto(layer.color)) {
                return makeError();
            }

            break;
        }

        case 420: {
            if (!parseInto(layer.trueColor)) {
                return makeError();
            }

            break;
        }

        case 370: {
            if (!parseInto(layer.lineweight)) {
                return makeError();
            }

            break;
        }

        case 6: {
            layer.lineType = m_data.value;

            break;
        }

        case 70: {
            const std::optional<int> maybeFlags{ parseAs<int>(m_data.value) };
            if (maybeFlags.has_value()) {
                layer.flags = static_cast<odxf::Layer::Flags>(*maybeFlags);
            } else {
                return parseError();
            }

            break;
        }

        case 1001: {
            xdataApp = m_data.value;

            break;
        }

        case 1071: {
            if (xdataApp != layerTransparencyApp) {
                break;
            }

            const std::optional<int> maybeTransparency{ parseAs<int>(m_data.value) };
            if (maybeTransparency.has_value()) {
                layer.transparency = decodeLayerTransparency(*maybeTransparency);
            } else {
                return parseError();
            }

            break;
        }
        }

        if (!readNext()) {
            return makeError();
        }
    }

    m_stream.layer(layer);

    return std::expected<void, Error>();
}

std::expected<void, Error> Reader::readBlocks()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return std::unexpected(
            Error{
                .lineNumber = m_currentLine,
                .what = "expected section begin",
            });
    }

    if (!readNext()) {
        return makeError();
    }

    if (!isBlocksBegin()) {
        return std::unexpected(
            Error{
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

std::expected<void, Error> Reader::readEntities()
{
    if (!readNext()) {
        return makeError();
    }

    if (!isSectionBegin()) {
        return std::unexpected(
            Error{
                .lineNumber = m_currentLine,
                .what = "expected section begin",
            });
    }

    if (!readNext()) {
        return makeError();
    }

    if (!isEntitiesBegin()) {
        return std::unexpected(
            Error{
                .lineNumber = m_currentLine,
                .what = "expected entities begin",
            });
    }

    while (!isSectionEnd()) {
        if (hasError()) {
            return makeError();
        }

        if (isLine()) {
            std::expected<void, Error> maybeResult{ readLine() };
            if (!maybeResult) {
                return maybeResult;
            }

        } else if (isCircle()) {
            std::expected<void, Error> maybeResult{ readCircle() };
            if (!maybeResult) {
                return maybeResult;
            }
        } else if (isArc()) {
            std::expected<void, Error> maybeResult{ readArc() };
            if (!maybeResult) {
                return maybeResult;
            }
        } else if (isLWPolyline()) {
            std::expected<void, Error> maybeResult{ readLWPolyline() };
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

template <EntityType E, std::predicate<E&> F>
std::expected<E, Error> Reader::readEntity(F readGroupCode)
{
    if (!readNext()) {
        return std::unexpected(m_error.value());
    }

    E entity;
    while (m_data.groupCode != 0) {
        if (!readGroupCode(entity) || !readNext()) {
            return std::unexpected(m_error.value());
        }
    }

    return entity;
}

bool Reader::readEntityProperty(EntityProperties& properties)
{
    switch (m_data.groupCode) {
    case 8: properties.layer = m_data.value; return true;

    case 62: return parseInto(properties.color);

    case 420: return parseInto(properties.trueColor);

    case 370: return parseInto(properties.lineweight);

    case 6: properties.lineType = m_data.value; return true;

    case 48: return parseInto(properties.lineTypeScale);

    case 440: {
        int transparency{ 0 };
        if (!parseInto(transparency)) {
            return false;
        }
        properties.transparency = decodeEntityTransparency(transparency);
        return true;
    }

    default:
        // unknown group codes are skipped
        return true;
    }
}

std::expected<void, Error> Reader::readLine()
{
    return readEntity<Line>([this](Line& line) -> bool {
               switch (m_data.groupCode) {
               case 10: return parseInto(line.start.x);
               case 20: return parseInto(line.start.y);
               case 11: return parseInto(line.end.x);
               case 21: return parseInto(line.end.y);
               default: return readEntityProperty(line.properties);
               }
           })
        .transform([this](const Line& line) { m_stream.line(line); });
}

std::expected<void, Error> Reader::readCircle()
{
    return readEntity<Circle>([this](Circle& circle) -> bool {
               switch (m_data.groupCode) {
               case 10: return parseInto(circle.center.x);
               case 20: return parseInto(circle.center.y);
               case 40: return parseInto(circle.radius);
               default: return readEntityProperty(circle.properties);
               }
           })
        .transform([this](const Circle& circle) { m_stream.circle(circle); });
}

std::expected<void, Error> Reader::readArc()
{
    return readEntity<Arc>([this](Arc& arc) -> bool {
               switch (m_data.groupCode) {
               case 10: return parseInto(arc.center.x);
               case 20: return parseInto(arc.center.y);
               case 40: return parseInto(arc.radius);
               case 50: return parseInto(arc.startAngle);
               case 51: return parseInto(arc.endAngle);
               default: return readEntityProperty(arc.properties);
               }
           })
        .transform([this](const Arc& arc) { m_stream.arc(arc); });
}

std::expected<void, Error> Reader::readLWPolyline()
{
    Vertex vertex;
    int numXY{ 0 };

    // A vertex is complete once it has x and y. Its bulge, if any, follows directly.
    const auto pushCompleteVertex = [&](LWPolyline& lwPolyline) {
        if (numXY == 2) {
            lwPolyline.vertices.push_back(vertex);
            numXY = 0;
            vertex.bulge.reset();
        }
    };

    std::expected<LWPolyline, Error> maybeLWPolyline{ readEntity<LWPolyline>(
        [&](LWPolyline& lwPolyline) -> bool {
            switch (m_data.groupCode) {
            case 90: {
                int verticesCount{ 0 };
                if (!parseInto(verticesCount)) {
                    return false;
                }
                if (verticesCount >= 0) {
                    lwPolyline.vertices.reserve(static_cast<std::size_t>(verticesCount));
                }
                return true;
            }

            case 70: {
                int flags{ 0 };
                if (!parseInto(flags)) {
                    return false;
                }
                lwPolyline.isClosed = (flags & 1) != 0;
                return true;
            }

            case 10:
                pushCompleteVertex(lwPolyline);
                if (!parseInto(vertex.position.x)) {
                    return false;
                }
                numXY++;
                return true;

            case 20:
                pushCompleteVertex(lwPolyline);
                if (!parseInto(vertex.position.y)) {
                    return false;
                }
                numXY++;
                return true;

            case 42:
                if (!parseInto(vertex.bulge)) {
                    return false;
                }
                pushCompleteVertex(lwPolyline);
                return true;

            default: return readEntityProperty(lwPolyline.properties);
            }
        }) };

    if (!maybeLWPolyline) {
        return std::unexpected(maybeLWPolyline.error());
    }

    pushCompleteVertex(*maybeLWPolyline);
    m_stream.lwPolyline(*maybeLWPolyline);

    return {};
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
        first, m_data.value.c_str() + m_data.value.size(), m_data.groupCode) };
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

std::expected<void, Error> Reader::makeError() const { return std::unexpected(m_error.value()); }

std::unexpected<Error> Reader::parseError() const
{
    return std::unexpected(Error{ .lineNumber = m_currentLine });
}

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

bool Reader::isLineType() const { return m_data.groupCode == 0 && m_data.value == "LTYPE"; }

bool Reader::isLine() const { return m_data.groupCode == 0 && m_data.value == "LINE"; }

bool odxf::Reader::isCircle() const { return m_data.groupCode == 0 && m_data.value == "CIRCLE"; }

bool Reader::isArc() const { return m_data.groupCode == 0 && m_data.value == "ARC"; }

bool Reader::isLWPolyline() const { return m_data.groupCode == 0 && m_data.value == "LWPOLYLINE"; }

bool Reader::isEOF() const { return m_data.groupCode == 0 && m_data.value == "EOF"; }

}   // namespace odxf
