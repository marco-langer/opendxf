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

void write_header(std::ofstream& stream, const odxf::header& header)
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

void write_line_type(std::ofstream& stream, const odxf::line_type& line_type)
{
    double pattern_length{ 0.0 };
    for (const odxf::line_type_element& element : line_type.elements) {
        pattern_length += std::abs(element.length);
    }

    stream << "\n0\nLTYPE";
    stream << "\n2\n" << line_type.name;
    stream << "\n70\n" << line_type.flags;
    stream << "\n3\n" << line_type.display_name;
    stream << "\n72\n65";
    stream << "\n73\n" << line_type.elements.size();
    stream << "\n40\n" << pattern_length;
    for (const odxf::line_type_element& element : line_type.elements) {
        stream << "\n49\n" << element.length;
        stream << "\n74\n" << element.complex_flags;
    }
}

void write_line_types(std::ofstream& stream, const std::vector<odxf::line_type>& line_types)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLTYPE";
    stream << "\n70\n" << line_types.size();
    for (const odxf::line_type& line_type : line_types) {
        write_line_type(stream, line_type);
    }
    stream << "\n0\nENDTAB";
}

void write_true_color(std::ofstream& stream, const std::optional<int>& maybe_true_color)
{
    if (maybe_true_color) {
        stream << "\n420\n" << *maybe_true_color;
    }
}

// BYLAYER is written as no group code 440.
void write_transparency(std::ofstream& stream, const odxf::entity_transparency& transparency)
{
    if (std::holds_alternative<odxf::by_block>(transparency)) {
        stream << "\n440\n" << odxf::by_block_transparency_value;
    } else if (const auto* by_value{ std::get_if<odxf::transparency>(&transparency) }) {
        stream << "\n440\n" << odxf::encode_transparency(*by_value);
    }
}

void write_layer(std::ofstream& stream, const odxf::layer& layer)
{
    stream << "\n0\nLAYER";
    stream << "\n2\n" << layer.name;
    stream << "\n70\n" << layer.flags;
    stream << "\n62\n" << layer.color;
    write_true_color(stream, layer.true_color);
    stream << "\n6\n" << layer.line_type;
    stream << "\n370\n" << layer.lineweight;
    // xdata comes last in a record
    if (layer.transparency) {
        stream << "\n1001\n" << odxf::layer_transparency_app;
        stream << "\n1071\n" << odxf::encode_transparency(*layer.transparency);
    }
}

void write_layers(std::ofstream& stream, const std::vector<odxf::layer>& layers)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLAYER";
    stream << "\n70\n" << layers.size();
    for (const odxf::layer& layer : layers) {
        write_layer(stream, layer);
    }
    stream << "\n0\nENDTAB";
}

// Every application that owns xdata in the file needs an entry in the APPID table.
void write_app_ids(std::ofstream& stream, const std::vector<odxf::layer>& layers)
{
    const bool has_layer_transparency{ std::ranges::any_of(
        layers, [](const odxf::layer& layer) { return layer.transparency.has_value(); }) };
    if (!has_layer_transparency) {
        return;
    }

    stream << "\n0\nTABLE";
    stream << "\n2\nAPPID";
    stream << "\n70\n1";
    stream << "\n0\nAPPID";
    stream << "\n2\n" << odxf::layer_transparency_app;
    stream << "\n70\n0";
    stream << "\n0\nENDTAB";
}

void write_tables(std::ofstream& stream, const odxf::tables& tables)
{
    stream << "\n0\nSECTION";
    stream << "\n2\nTABLES";
    write_line_types(stream, tables.line_types);
    write_layers(stream, tables.layers);
    write_app_ids(stream, tables.layers);
    stream << "\n0\nENDSEC";
}

void write_blocks(std::ofstream& stream)
{
    stream << "\n0\nSECTION";
    stream << "\n2\nBLOCKS";
    stream << "\n0\nENDSEC";
}

void write_coordinate(std::ofstream& stream, const odxf::coordinate3d& coordinate)
{
    stream << "\n10\n" << std::to_string(coordinate.x);
    stream << "\n20\n" << std::to_string(coordinate.y);
    stream << "\n30\n" << std::to_string(coordinate.z);
}

void write_thickness(std::ofstream& stream, const std::optional<double>& maybe_thickness)
{
    if (maybe_thickness) {
        stream << "\n39\n" << *maybe_thickness;
    }
}

void write_extrusion(std::ofstream& stream, const std::optional<odxf::vector3d>& maybe_extrusion)
{
    if (maybe_extrusion) {
        const odxf::vector3d& extrusion{ *maybe_extrusion };
        stream << "\n210\n" << std::to_string(extrusion.x);
        stream << "\n220\n" << std::to_string(extrusion.y);
        stream << "\n230\n" << std::to_string(extrusion.z);
    }
}

void write_entity_properties(std::ofstream& stream, const odxf::entity_properties& properties)
{
    stream << "\n8\n" << properties.layer;
    stream << "\n6\n" << properties.line_type;
    stream << "\n62\n" << properties.color;
    stream << "\n370\n" << properties.lineweight;
    stream << "\n48\n" << properties.line_type_scale;
    write_true_color(stream, properties.true_color);
    write_transparency(stream, properties.transparency);
}

void write_entity(std::ofstream& stream, const odxf::point& point)
{
    stream << "\n0\nPOINT\n100\nAcDbPoint";
    write_entity_properties(stream, point.properties);
    write_thickness(stream, point.thickness);
    stream << "\n10\n" << std::to_string(point.coordinate.x);
    stream << "\n20\n" << std::to_string(point.coordinate.y);
    stream << "\n30\n" << std::to_string(point.coordinate.z);
    write_extrusion(stream, point.extrusion);
}


void write_entity(std::ofstream& stream, const odxf::ray& ray)
{
    stream << "\n0\nRAY\n100\nAcDbRay";
    write_entity_properties(stream, ray.properties);
    stream << "\n10\n" << std::to_string(ray.start_point.x);
    stream << "\n20\n" << std::to_string(ray.start_point.y);
    stream << "\n30\n" << std::to_string(ray.start_point.z);
    stream << "\n11\n" << std::to_string(ray.direction.x);
    stream << "\n21\n" << std::to_string(ray.direction.y);
    stream << "\n31\n" << std::to_string(ray.direction.z);
}

void write_entity(std::ofstream& stream, const odxf::line& line)
{
    stream << "\n0\nLINE\n100\nAcDbLine";
    write_entity_properties(stream, line.properties);
    write_thickness(stream, line.thickness);
    stream << "\n10\n" << std::to_string(line.start.x);
    stream << "\n20\n" << std::to_string(line.start.y);
    stream << "\n30\n" << std::to_string(line.start.z);
    stream << "\n11\n" << std::to_string(line.end.x);
    stream << "\n21\n" << std::to_string(line.end.y);
    stream << "\n31\n" << std::to_string(line.end.z);
    write_extrusion(stream, line.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::circle& circle)
{
    stream << "\n0\nCIRCLE\n100\nAcDbCircle";
    write_entity_properties(stream, circle.properties);
    write_thickness(stream, circle.thickness);
    stream << "\n10\n" << std::to_string(circle.center.x);
    stream << "\n20\n" << std::to_string(circle.center.y);
    stream << "\n30\n" << std::to_string(circle.center.z);
    stream << "\n40\n" << std::to_string(circle.radius);
    write_extrusion(stream, circle.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::arc& arc)
{
    stream << "\n0\nARC\n100\nAcDbCircle";
    write_entity_properties(stream, arc.properties);
    write_thickness(stream, arc.thickness);
    stream << "\n10\n" << std::to_string(arc.center.x);
    stream << "\n20\n" << std::to_string(arc.center.y);
    stream << "\n30\n" << std::to_string(arc.center.z);
    stream << "\n40\n" << std::to_string(arc.radius);
    stream << "\n100\nAcDbArc";
    stream << "\n50\n" << std::to_string(arc.start_angle);
    stream << "\n51\n" << std::to_string(arc.end_angle);
    write_extrusion(stream, arc.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::ellipse& ellipse)
{
    stream << "\n0\nELLIPSE\n100\nAcDbEllipse";
    write_entity_properties(stream, ellipse.properties);
    stream << "\n10\n" << std::to_string(ellipse.center.x);
    stream << "\n20\n" << std::to_string(ellipse.center.y);
    stream << "\n30\n" << std::to_string(ellipse.center.z);
    stream << "\n11\n" << std::to_string(ellipse.end_point_major.x);
    stream << "\n21\n" << std::to_string(ellipse.end_point_major.y);
    stream << "\n31\n" << std::to_string(ellipse.end_point_major.z);
    stream << "\n40\n" << std::to_string(ellipse.axis_ratio);
    stream << "\n41\n" << std::to_string(ellipse.start_parameter);
    stream << "\n42\n" << std::to_string(ellipse.end_parameter);
    write_extrusion(stream, ellipse.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::lw_polyline& lw_polyline)
{
    stream << "\n0\nLWPOLYLINE\n100\nAcDbPolyline";
    write_entity_properties(stream, lw_polyline.properties);
    stream << "\n90\n" << lw_polyline.vertices.size();
    stream << "\n70\n" << (lw_polyline.is_closed ? 1 : 0);

    for (const odxf::vertex& vertex : lw_polyline.vertices) {
        stream << "\n10\n" << std::to_string(vertex.position.x);
        stream << "\n20\n" << std::to_string(vertex.position.y);
        if (vertex.bulge.has_value()) {
            stream << "\n42\n" << std::to_string(*vertex.bulge);
        }
    }
}

std::expected<void, odxf::error> check_stream(const std::ofstream& stream)
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
write(const std::filesystem::path& file_path, const odxf::document& document)
{
    std::expected<write_stream, error> maybe_stream{ write_stream::open(
        file_path, document.header, document.tables) };
    if (!maybe_stream) {
        return std::unexpected(maybe_stream.error());
    }

    for (const odxf::entity& entity : document.entities) {
        if (std::expected<void, error> maybe_error = maybe_stream->entity(entity); !maybe_error) {
            return maybe_error;
        }
    }

    return maybe_stream->close();
}

std::expected<write_stream, error> write_stream::open(
    const std::filesystem::path& file_path, const odxf::header& header, const odxf::tables& tables)
{
    std::ofstream stream{ file_path };
    if (!stream.is_open()) {
        return std::unexpected(
            error{
                .type = error_type::file_open_error,
                .what = std::format(
                    "unable to open file {}",
                    reinterpret_cast<const char*>(file_path.u8string().c_str())),
            });
    }

    stream << std::setprecision(16);

    write_header(stream, header);
    write_tables(stream, tables);
    write_blocks(stream);
    stream << "\n0\nSECTION\n2\nENTITIES";

    if (std::expected<void, error> maybe_error = check_stream(stream); !maybe_error) {
        return std::unexpected(maybe_error.error());
    }

    return write_stream{ std::move(stream) };
}

write_stream::write_stream(std::ofstream stream)
    : m_stream{ std::move(stream) }
{
}

write_stream::write_stream(write_stream&& other) noexcept
    : m_stream{ std::move(other.m_stream) }
    , m_is_closed{ std::exchange(other.m_is_closed, true) }
{
}

odxf::write_stream& write_stream::operator=(write_stream&& other) noexcept
{
    if (this != &other) {
        if (!m_is_closed) {
            static_cast<void>(close());
        }
        m_stream = std::move(other.m_stream);
        m_is_closed = std::exchange(other.m_is_closed, true);
    }

    return *this;
}

write_stream::~write_stream()
{
    if (!m_is_closed) {
        static_cast<void>(close());
    }
}

std::expected<void, error> write_stream::entity(const odxf::entity& entity)
{
    if (m_is_closed) {
        return std::unexpected(
            error{
                .type = error_type::write_error,
                .what = "entity written after close",
            });
    }

    std::visit([this](const auto& e) { write_entity(m_stream, e); }, entity);

    return check_stream(m_stream);
}

std::expected<void, error> write_stream::close()
{
    if (m_is_closed) {
        return std::unexpected(
            error{
                .type = error_type::write_error,
                .what = "close called twice",
            });
    }

    m_is_closed = true;
    m_stream << "\n0\nENDSEC";
    m_stream << "\n0\nEOF";
    m_stream.close();

    return check_stream(m_stream);
}

}   // namespace odxf
