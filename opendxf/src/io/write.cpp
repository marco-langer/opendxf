// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/io/write.hpp"

#include "overload.hpp"
#include "standard_arithmetic.hpp"
#include "transparency.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <format>
#include <fstream>
#include <system_error>
#include <utility>

namespace {

// DXF numbers must not depend on the locale, which operator<< takes from the stream. Doubles get
// the shortest representation that reads back as the same value. A formatting error fails the
// stream, so check_stream() reports it like any other write error.
template <odxf::standard_arithmetic T>
void write_number(std::ofstream& stream, T value)
{
    std::array<char, 32> buffer;
    const auto [end, ec]{ std::to_chars(buffer.data(), buffer.data() + buffer.size(), value) };
    if (ec != std::errc{}) {
        stream.setstate(std::ios::failbit);
        return;
    }

    stream.write(buffer.data(), end - buffer.data());
}

template <typename T>
void write_value(std::ofstream& stream, int group_code, T value)
{
    stream << '\n';
    write_number(stream, group_code);
    stream << '\n';
    write_number(stream, value);
}

void write_header(std::ofstream& stream, const odxf::header& header)
{
    stream << "999\nopendxf";
    stream << "\n0\nSECTION";
    stream << "\n2\nHEADER";

    for (const auto& [key, value] : header.entries) {
        stream << "\n9\n" << key;
        std::visit(
            odxf::overload{ [&stream, &key](bool element) {
                               write_value(stream, 290, static_cast<int>(element));
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
                                write_number(stream, element);
                            },
                            [&stream, &key](double element) {
                                write_value(stream, key == "$ANGBASE" ? 50 : 40, element);
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
                                write_value(stream, 10, coord.x);
                                write_value(stream, 20, coord.y);
                            },
                            [&stream](const odxf::coordinate3d& coord) {
                                write_value(stream, 10, coord.x);
                                write_value(stream, 20, coord.y);
                                write_value(stream, 30, coord.z);
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
    write_value(stream, 70, line_type.flags);
    stream << "\n3\n" << line_type.display_name;
    stream << "\n72\n65";
    write_value(stream, 73, line_type.elements.size());
    write_value(stream, 40, pattern_length);
    for (const odxf::line_type_element& element : line_type.elements) {
        write_value(stream, 49, element.length);
        write_value(stream, 74, element.complex_flags);
    }
}

void write_line_types(std::ofstream& stream, const std::vector<odxf::line_type>& line_types)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLTYPE";
    write_value(stream, 70, line_types.size());
    for (const odxf::line_type& line_type : line_types) {
        write_line_type(stream, line_type);
    }
    stream << "\n0\nENDTAB";
}

void write_true_color(std::ofstream& stream, const std::optional<int>& maybe_true_color)
{
    if (maybe_true_color) {
        write_value(stream, 420, *maybe_true_color);
    }
}

// BYLAYER is written as no group code 440.
void write_transparency(std::ofstream& stream, const odxf::entity_transparency& transparency)
{
    if (std::holds_alternative<odxf::by_block>(transparency)) {
        write_value(stream, 440, odxf::by_block_transparency_value);
    } else if (const auto* by_value{ std::get_if<odxf::transparency>(&transparency) }) {
        write_value(stream, 440, odxf::encode_transparency(*by_value));
    }
}

void write_layer(std::ofstream& stream, const odxf::layer& layer)
{
    stream << "\n0\nLAYER";
    stream << "\n2\n" << layer.name;
    write_value(stream, 70, std::to_underlying(layer.flags));
    write_value(stream, 62, layer.color);
    write_true_color(stream, layer.true_color);
    stream << "\n6\n" << layer.line_type;
    write_value(stream, 370, layer.lineweight);
    // xdata comes last in a record
    if (layer.transparency) {
        stream << "\n1001\n" << odxf::layer_transparency_app;
        write_value(stream, 1071, odxf::encode_transparency(*layer.transparency));
    }
}

void write_layers(std::ofstream& stream, const std::vector<odxf::layer>& layers)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLAYER";
    write_value(stream, 70, layers.size());
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
    write_value(stream, 10, coordinate.x);
    write_value(stream, 20, coordinate.y);
    write_value(stream, 30, coordinate.z);
}

void write_thickness(std::ofstream& stream, const std::optional<double>& maybe_thickness)
{
    if (maybe_thickness) {
        write_value(stream, 39, *maybe_thickness);
    }
}

void write_extrusion(std::ofstream& stream, const std::optional<odxf::vector3d>& maybe_extrusion)
{
    if (maybe_extrusion) {
        const odxf::vector3d& extrusion{ *maybe_extrusion };
        write_value(stream, 210, extrusion.x);
        write_value(stream, 220, extrusion.y);
        write_value(stream, 230, extrusion.z);
    }
}

void write_entity_properties(std::ofstream& stream, const odxf::entity_properties& properties)
{
    stream << "\n8\n" << properties.layer;
    stream << "\n6\n" << properties.line_type;
    write_value(stream, 62, properties.color);
    write_value(stream, 370, properties.lineweight);
    write_value(stream, 48, properties.line_type_scale);
    write_true_color(stream, properties.true_color);
    write_transparency(stream, properties.transparency);
    if (!properties.is_visible) {
        write_value(stream, 60, 1);
    }
}

void write_entity(std::ofstream& stream, const odxf::point& point)
{
    stream << "\n0\nPOINT\n100\nAcDbPoint";
    write_entity_properties(stream, point.properties);
    write_thickness(stream, point.thickness);
    write_value(stream, 10, point.coordinate.x);
    write_value(stream, 20, point.coordinate.y);
    write_value(stream, 30, point.coordinate.z);
    write_extrusion(stream, point.extrusion);
}


void write_entity(std::ofstream& stream, const odxf::ray& ray)
{
    stream << "\n0\nRAY\n100\nAcDbRay";
    write_entity_properties(stream, ray.properties);
    write_value(stream, 10, ray.start_point.x);
    write_value(stream, 20, ray.start_point.y);
    write_value(stream, 30, ray.start_point.z);
    write_value(stream, 11, ray.direction.x);
    write_value(stream, 21, ray.direction.y);
    write_value(stream, 31, ray.direction.z);
}

void write_entity(std::ofstream& stream, const odxf::line& line)
{
    stream << "\n0\nLINE\n100\nAcDbLine";
    write_entity_properties(stream, line.properties);
    write_thickness(stream, line.thickness);
    write_value(stream, 10, line.start.x);
    write_value(stream, 20, line.start.y);
    write_value(stream, 30, line.start.z);
    write_value(stream, 11, line.end.x);
    write_value(stream, 21, line.end.y);
    write_value(stream, 31, line.end.z);
    write_extrusion(stream, line.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::circle& circle)
{
    stream << "\n0\nCIRCLE\n100\nAcDbCircle";
    write_entity_properties(stream, circle.properties);
    write_thickness(stream, circle.thickness);
    write_value(stream, 10, circle.center.x);
    write_value(stream, 20, circle.center.y);
    write_value(stream, 30, circle.center.z);
    write_value(stream, 40, circle.radius);
    write_extrusion(stream, circle.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::arc& arc)
{
    stream << "\n0\nARC\n100\nAcDbCircle";
    write_entity_properties(stream, arc.properties);
    write_thickness(stream, arc.thickness);
    write_value(stream, 10, arc.center.x);
    write_value(stream, 20, arc.center.y);
    write_value(stream, 30, arc.center.z);
    write_value(stream, 40, arc.radius);
    stream << "\n100\nAcDbArc";
    write_value(stream, 50, arc.start_angle);
    write_value(stream, 51, arc.end_angle);
    write_extrusion(stream, arc.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::ellipse& ellipse)
{
    stream << "\n0\nELLIPSE\n100\nAcDbEllipse";
    write_entity_properties(stream, ellipse.properties);
    write_value(stream, 10, ellipse.center.x);
    write_value(stream, 20, ellipse.center.y);
    write_value(stream, 30, ellipse.center.z);
    write_value(stream, 11, ellipse.end_point_major.x);
    write_value(stream, 21, ellipse.end_point_major.y);
    write_value(stream, 31, ellipse.end_point_major.z);
    write_value(stream, 40, ellipse.axis_ratio);
    write_value(stream, 41, ellipse.start_parameter);
    write_value(stream, 42, ellipse.end_parameter);
    write_extrusion(stream, ellipse.extrusion);
}

void write_entity(std::ofstream& stream, const odxf::lw_polyline& lw_polyline)
{
    stream << "\n0\nLWPOLYLINE\n100\nAcDbPolyline";
    write_entity_properties(stream, lw_polyline.properties);
    write_value(stream, 90, lw_polyline.vertices.size());
    write_value(stream, 70, lw_polyline.is_closed ? 1 : 0);

    for (const odxf::vertex& vertex : lw_polyline.vertices) {
        write_value(stream, 10, vertex.position.x);
        write_value(stream, 20, vertex.position.y);
        if (vertex.bulge.has_value()) {
            write_value(stream, 42, *vertex.bulge);
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
