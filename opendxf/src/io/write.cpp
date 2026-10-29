// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/write.hpp"

#include "overload.hpp"
#include "transparency.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
#include <iomanip>
#include <utility>

namespace {

void writeHeader(std::ofstream& stream, const odxf::header& header)
{
    stream << "999\nopendxf";
    stream << "\n0\nSECTION";
    stream << "\n2\nHEADER";

    for (const auto& [key, value] : header.entries) {
        stream << "\n9\n" << key;
        std::visit(
            odxf::overload{ [&stream, &key](bool element) {
                               stream << "\n290\n";
                               stream << element;
                           },
                            [&stream, &key](int element) {
                                if (key == "$CECOLOR" || key == "$INTERFERECOLOR") {
                                    stream << "\n62\n";
                                } else if (
                                    key == "$ENDCAPS" || key == "$JOINSTYLE" || key == "$SORTENTS"
                                    || key == "$INDEXCTL" || key == "$HIDETEXT" || key == "$HALOGAP"
                                    || key == "$OBSLTYPE" || key == "$INTERSECTIONDISPLAY"
                                    || key == "$DIMASSOC" || key == "$LOFTNORMALS"
                                    || key == "$LIGHTGLYPHDISPLAY" || key == "$TILEMODELIGHTSYNCH"
                                    || key == "$SOLIDHIST" || key == "$SHOWHIST"
                                    || key == "$DWFFRAME" || key == "$DGNFRAME"
                                    || key == "$CSHADOW") {
                                    stream << "\n280\n";
                                } else if (key == "$CELWEIGHT") {
                                    stream << "\n370\n";
                                } else if (key == "$CEPSNTYPE") {
                                    stream << "\n380\n";
                                } else {
                                    stream << "\n70\n";
                                }
                                stream << element;
                            },
                            [&stream, &key](double element) {
                                if (key == "$ANGBASE") {
                                    stream << "\n50\n";
                                } else {
                                    stream << "\n40\n";
                                }
                                stream << element;
                            },
                            [&stream, &key](const std::string& element) {
                                if (key == "$DIMSTYLE") {
                                    stream << "\n2\n";
                                } else if (key == "$HANDSEED") {
                                    stream << "\n5\n";
                                } else if (
                                    key == "$CELTYPE" || key == "$DIMLTYPE" || key == "$DIMLTEX1"
                                    || key == "$DIMLTEX2") {
                                    stream << "\n6\n";
                                } else if (key == "$TEXTSTYLE" || key == "$DIMTXSTY") {
                                    stream << "\n7\n";
                                } else if (key == "$CLAYER") {
                                    stream << "\n8\n";
                                } else {
                                    stream << "\n1\n";
                                }
                                stream << element;
                            },
                            [&stream](const odxf::coordinate2d& coord) {
                                stream << "\n10\n";
                                stream << coord.x;
                                stream << "\n20\n";
                                stream << coord.y;
                            },
                            [&stream](const odxf::coordinate3d& coord) {
                                stream << "\n10\n";
                                stream << coord.x;
                                stream << "\n20\n";
                                stream << coord.y;
                                stream << "\n30\n";
                                stream << coord.z;
                            } },
            value);
    }

    stream << "\n0\nENDSEC";
}

void writeLineType(std::ofstream& stream, const odxf::line_type& lineType)
{
    double patternLength{ 0.0 };
    for (const odxf::line_type_element& element : lineType.elements) {
        patternLength += std::abs(element.length);
    }

    stream << "\n0\nLTYPE";
    stream << "\n2\n" << lineType.name;
    stream << "\n70\n" << lineType.flags;
    stream << "\n3\n" << lineType.displayName;
    stream << "\n72\n65";
    stream << "\n73\n" << lineType.elements.size();
    stream << "\n40\n" << patternLength;
    for (const odxf::line_type_element& element : lineType.elements) {
        stream << "\n49\n" << element.length;
        stream << "\n74\n" << element.complexFlags;
    }
}

void writeLineTypes(std::ofstream& stream, const std::vector<odxf::line_type>& lineTypes)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLTYPE";
    stream << "\n70\n" << lineTypes.size();
    for (const odxf::line_type& lineType : lineTypes) {
        writeLineType(stream, lineType);
    }
    stream << "\n0\nENDTAB";
}

void writeTrueColor(std::ofstream& stream, const std::optional<int>& maybeTrueColor)
{
    if (maybeTrueColor) {
        stream << "\n420\n" << *maybeTrueColor;
    }
}

// BYLAYER is written as no group code 440.
void writeTransparency(std::ofstream& stream, const odxf::entity_transparency& transparency)
{
    if (std::holds_alternative<odxf::by_block>(transparency)) {
        stream << "\n440\n" << odxf::byBlockTransparencyValue;
    } else if (const auto* byValue{ std::get_if<odxf::transparency>(&transparency) }) {
        stream << "\n440\n" << odxf::encodeTransparency(*byValue);
    }
}

void writeLayer(std::ofstream& stream, const odxf::layer& layer)
{
    stream << "\n0\nLAYER";
    stream << "\n2\n" << layer.name;
    stream << "\n70\n" << layer.flags;
    stream << "\n62\n" << layer.color;
    writeTrueColor(stream, layer.trueColor);
    stream << "\n6\n" << layer.lineType;
    stream << "\n370\n" << layer.lineweight;
    // xdata comes last in a record
    if (layer.transparency) {
        stream << "\n1001\n" << odxf::layerTransparencyApp;
        stream << "\n1071\n" << odxf::encodeTransparency(*layer.transparency);
    }
}

void writeLayers(std::ofstream& stream, const std::vector<odxf::layer>& layers)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLAYER";
    stream << "\n70\n" << layers.size();
    for (const odxf::layer& layer : layers) {
        writeLayer(stream, layer);
    }
    stream << "\n0\nENDTAB";
}

// Every application that owns xdata in the file needs an entry in the APPID table.
void writeAppIds(std::ofstream& stream, const std::vector<odxf::layer>& layers)
{
    const bool hasLayerTransparency{ std::ranges::any_of(
        layers, [](const odxf::layer& layer) { return layer.transparency.has_value(); }) };
    if (!hasLayerTransparency) {
        return;
    }

    stream << "\n0\nTABLE";
    stream << "\n2\nAPPID";
    stream << "\n70\n1";
    stream << "\n0\nAPPID";
    stream << "\n2\n" << odxf::layerTransparencyApp;
    stream << "\n70\n0";
    stream << "\n0\nENDTAB";
}

void writeTables(std::ofstream& stream, const odxf::tables& tables)
{
    stream << "\n0\nSECTION";
    stream << "\n2\nTABLES";
    writeLineTypes(stream, tables.lineTypes);
    writeLayers(stream, tables.layers);
    writeAppIds(stream, tables.layers);
    stream << "\n0\nENDSEC";
}

void writeBlocks(std::ofstream& stream)
{
    stream << "\n0\nSECTION";
    stream << "\n2\nBLOCKS";
    stream << "\n0\nENDSEC";
}

void writeCoordinate(std::ofstream& stream, const odxf::coordinate3d& coordinate)
{
    stream << "\n10\n" << std::to_string(coordinate.x);
    stream << "\n20\n" << std::to_string(coordinate.y);
    stream << "\n30\n" << std::to_string(coordinate.z);
}

void writeThickness(std::ofstream& stream, const std::optional<double>& maybeThickness)
{
    if (maybeThickness) {
        stream << "\n39\n" << *maybeThickness;
    }
}

void writeExtrusion(std::ofstream& stream, const std::optional<odxf::vector3d>& maybeExtrusion)
{
    if (maybeExtrusion) {
        const odxf::vector3d& extrusion{ *maybeExtrusion };
        stream << "\n210\n" << std::to_string(extrusion.x);
        stream << "\n220\n" << std::to_string(extrusion.y);
        stream << "\n230\n" << std::to_string(extrusion.z);
    }
}

void writeEntityProperties(std::ofstream& stream, const odxf::entity_properties& properties)
{
    stream << "\n8\n" << properties.layer;
    stream << "\n6\n" << properties.lineType;
    stream << "\n62\n" << properties.color;
    stream << "\n370\n" << properties.lineweight;
    stream << "\n48\n" << properties.lineTypeScale;
    writeTrueColor(stream, properties.trueColor);
    writeTransparency(stream, properties.transparency);
}

void writeEntity(std::ofstream& stream, const odxf::point& point)
{
    stream << "\n0\nLINE\n100\nAcDbPoint";
    writeEntityProperties(stream, point.properties);
    writeThickness(stream, point.thickness);
    stream << "\n10\n" << std::to_string(point.coordinate.x);
    stream << "\n20\n" << std::to_string(point.coordinate.y);
    stream << "\n30\n" << std::to_string(point.coordinate.z);
    writeExtrusion(stream, point.extrusion);
}


void writeEntity(std::ofstream& stream, const odxf::ray& ray)
{
    stream << "\n0\nLINE\n100\nAcDbRay";
    writeEntityProperties(stream, ray.properties);
    stream << "\n10\n" << std::to_string(ray.startPoint.x);
    stream << "\n20\n" << std::to_string(ray.startPoint.y);
    stream << "\n30\n" << std::to_string(ray.startPoint.z);
    stream << "\n11\n" << std::to_string(ray.direction.x);
    stream << "\n21\n" << std::to_string(ray.direction.y);
    stream << "\n31\n" << std::to_string(ray.direction.z);
}

void writeEntity(std::ofstream& stream, const odxf::line& line)
{
    stream << "\n0\nLINE\n100\nAcDbLine";
    writeEntityProperties(stream, line.properties);
    writeThickness(stream, line.thickness);
    stream << "\n10\n" << std::to_string(line.start.x);
    stream << "\n20\n" << std::to_string(line.start.y);
    stream << "\n30\n" << std::to_string(line.start.z);
    stream << "\n11\n" << std::to_string(line.end.x);
    stream << "\n21\n" << std::to_string(line.end.y);
    stream << "\n31\n" << std::to_string(line.end.z);
    writeExtrusion(stream, line.extrusion);
}

void writeEntity(std::ofstream& stream, const odxf::circle& circle)
{
    stream << "\n0\nCIRCLE\n100\nAcDbCircle";
    writeEntityProperties(stream, circle.properties);
    writeThickness(stream, circle.thickness);
    stream << "\n10\n" << std::to_string(circle.center.x);
    stream << "\n20\n" << std::to_string(circle.center.y);
    stream << "\n30\n" << std::to_string(circle.center.z);
    stream << "\n40\n" << std::to_string(circle.radius);
    writeExtrusion(stream, circle.extrusion);
}

void writeEntity(std::ofstream& stream, const odxf::arc& arc)
{
    stream << "\n0\nARC\n100\nAcDbCircle";
    writeEntityProperties(stream, arc.properties);
    writeThickness(stream, arc.thickness);
    stream << "\n10\n" << std::to_string(arc.center.x);
    stream << "\n20\n" << std::to_string(arc.center.y);
    stream << "\n30\n" << std::to_string(arc.center.z);
    stream << "\n40\n" << std::to_string(arc.radius);
    stream << "\n100\nAcDbArc";
    stream << "\n50\n" << std::to_string(arc.startAngle);
    stream << "\n51\n" << std::to_string(arc.endAngle);
    writeExtrusion(stream, arc.extrusion);
}

void writeEntity(std::ofstream& stream, const odxf::ellipse& ellipse)
{
    stream << "\n0\nARC\n100\nAcDbEllipse";
    writeEntityProperties(stream, ellipse.properties);
    stream << "\n10\n" << std::to_string(ellipse.center.x);
    stream << "\n20\n" << std::to_string(ellipse.center.y);
    stream << "\n30\n" << std::to_string(ellipse.center.z);
    stream << "\n11\n" << std::to_string(ellipse.endPointMajor.x);
    stream << "\n21\n" << std::to_string(ellipse.endPointMajor.y);
    stream << "\n31\n" << std::to_string(ellipse.endPointMajor.z);
    stream << "\n40\n" << std::to_string(ellipse.axisRatio);
    stream << "\n41\n" << std::to_string(ellipse.startParameter);
    stream << "\n42\n" << std::to_string(ellipse.endParameter);
    writeExtrusion(stream, ellipse.extrusion);
}

void writeEntity(std::ofstream& stream, const odxf::lw_polyline& lwPolyline)
{
    stream << "\n0\nLWPOLYLINE\n100\nAcDbPolyline";
    writeEntityProperties(stream, lwPolyline.properties);
    stream << "\n90\n" << lwPolyline.vertices.size();
    stream << "\n70\n" << lwPolyline.isClosed ? 1 : 0;

    for (const odxf::vertex& vertex : lwPolyline.vertices) {
        stream << "\n10\n" << std::to_string(vertex.position.x);
        stream << "\n20\n" << std::to_string(vertex.position.y);
        if (vertex.bulge.has_value()) {
            stream << "\n42\n" << std::to_string(*vertex.bulge);
        }
    }
}

std::expected<void, odxf::error> checkStream(const std::ofstream& stream)
{
    if (!stream) {
        return std::unexpected(
            odxf::error{
                .type = odxf::error_type::write_error,
                .what = "unable to write the file",
            });
    }

    return {};
}

}   // namespace

namespace odxf {

std::expected<void, error>
write(const std::filesystem::path& filePath, const odxf::document& document)
{
    std::expected<write_stream, error> maybeStream{ write_stream::open(
        filePath, document.header, document.tables) };
    if (!maybeStream) {
        return std::unexpected(maybeStream.error());
    }

    for (const odxf::entity& entity : document.entities) {
        if (std::expected<void, error> maybeError = maybeStream->entity(entity); !maybeError) {
            return maybeError;
        }
    }

    return maybeStream->close();
}

std::expected<write_stream, error> write_stream::open(
    const std::filesystem::path& filePath, const odxf::header& header, const odxf::tables& tables)
{
    std::ofstream stream{ filePath };
    if (!stream.is_open()) {
        return std::unexpected(
            error{
                .type = error_type::file_open_error,
                .what = std::format(
                    "unable to open file {}",
                    reinterpret_cast<const char*>(filePath.u8string().c_str())),
            });
    }

    stream << std::setprecision(16);

    writeHeader(stream, header);
    writeTables(stream, tables);
    writeBlocks(stream);
    stream << "\n0\nSECTION\n2\nENTITIES";

    if (std::expected<void, error> maybeError = checkStream(stream); !maybeError) {
        return std::unexpected(maybeError.error());
    }

    return write_stream{ std::move(stream) };
}

write_stream::write_stream(std::ofstream stream)
    : m_stream{ std::move(stream) }
{
}

write_stream::write_stream(write_stream&& other) noexcept
    : m_stream{ std::move(other.m_stream) }
    , m_isClosed{ std::exchange(other.m_isClosed, true) }
{
}

odxf::write_stream& write_stream::operator=(write_stream&& other) noexcept
{
    if (this != &other) {
        if (!m_isClosed) {
            static_cast<void>(close());
        }
        m_stream = std::move(other.m_stream);
        m_isClosed = std::exchange(other.m_isClosed, true);
    }

    return *this;
}

write_stream::~write_stream()
{
    if (!m_isClosed) {
        static_cast<void>(close());
    }
}

std::expected<void, error> write_stream::entity(const odxf::entity& entity)
{
    if (m_isClosed) {
        return std::unexpected(
            error{
                .type = error_type::write_error,
                .what = "entity written after close",
            });
    }

    std::visit([this](const auto& e) { writeEntity(m_stream, e); }, entity);

    return checkStream(m_stream);
}

std::expected<void, error> write_stream::close()
{
    if (m_isClosed) {
        return std::unexpected(
            error{
                .type = error_type::write_error,
                .what = "close called twice",
            });
    }

    m_isClosed = true;
    m_stream << "\n0\nENDSEC";
    m_stream << "\n0\nEOF";
    m_stream.close();

    return checkStream(m_stream);
}

}   // namespace odxf
