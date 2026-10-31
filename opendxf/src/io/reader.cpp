// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "reader.hpp"
#include "parse.hpp"
#include "transparency.hpp"

#include "opendxf/io/read_stream_base.hpp"
#include "opendxf/layer.hpp"
#include "opendxf/tables.hpp"

#include <concepts>
#include <format>
#include <optional>

namespace odxf {

namespace {

    // An entity with an optional extrusion direction (group codes 210, 220 and 230).
    template <typename E>
    concept extruded_entity = entity_type<E> && requires(E& entity) {
        { entity.extrusion } -> std::same_as<std::optional<vector3d>&>;
    };

    // The extrusion is optional, but its group codes 210, 220 and 230 come one by one.
    template <extruded_entity E>
    vector3d& extrusion(E& entity)
    {
        if (!entity.extrusion) {
            entity.extrusion.emplace();
        }
        return *entity.extrusion;
    }

}   // namespace

template <typename T>
bool reader::parse_into(T& target)
{
    const std::optional<T> maybe_value{ parse_as<T>(m_data.value) };
    if (!maybe_value) {
        m_error = parse_error().error();
        return false;
    }

    target = *maybe_value;
    return true;
}

template <typename T>
bool reader::parse_into(std::optional<T>& target)
{
    T value;
    if (!parse_into(value)) {
        return false;
    }

    target = value;
    return true;
}

reader::reader(read_stream_base& stream)
    : m_stream{ stream }
{
}

std::expected<void, error> reader::read_all(const std::filesystem::path& file_path)
{
    if (!open(file_path)) {
        return make_error();
    }

    if (std::expected<void, error> maybe_error = read_header(); !maybe_error) {
        return maybe_error;
    }

    if (std::expected<void, error> maybe_error = read_tables(); !maybe_error) {
        return maybe_error;
    }

    if (std::expected<void, error> maybe_error = read_blocks(); !maybe_error) {
        return maybe_error;
    }

    if (std::expected<void, error> maybe_error = read_entities(); !maybe_error) {
        return maybe_error;
    }

    if (!read_next()) {
        return make_error();
    }

    if (is_eof()) {
        return {};
    }

    return std::unexpected(error{ .line_number = m_current_line, .what = "EOF missing" });
}

bool reader::open(const std::filesystem::path& file_path)
{
    m_file_stream.open(file_path, std::ios::binary);

    if (!m_file_stream.is_open()) {
        m_error = error{
            .type = error_type::file_open_error,
            .what = std::format(
                "unable to open file {}",
                reinterpret_cast<const char*>(file_path.u8string().c_str())),
        };

        return false;
    }

    return true;
}

std::expected<void, error> reader::read_header()
{
    if (!read_next()) {
        return make_error();
    }

    if (!is_section_begin()) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected section begin",
            });
    }

    if (!read_next()) {
        return make_error();
    }

    if (!is_header_begin()) {
        return {};   // header is optional
    }

    if (!read_next()) {
        return make_error();
    }

    odxf::header header;
    while (!is_section_end()) {
        if (has_error()) {
            return std::unexpected(m_error.value());
        }

        std::expected<header_entry, error> maybe_header_entry{ read_header_entry() };
        if (!maybe_header_entry) {
            return std::unexpected(maybe_header_entry.error());
        }

        odxf::header_entry& header_entry{ *maybe_header_entry };
        header.entries.emplace(header_entry);
    }

    m_stream.header(header);

    return {};
}

std::expected<header_entry, error> reader::read_header_entry()
{
    if (m_data.group_code != 9) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected group code 9 for header variable name",
            });
    }

    header_key key{ m_data.value };

    if (!read_next()) {
        return std::unexpected(m_error.value());
    }

    switch (m_data.group_code) {
    case 1:
    case 2:
    case 3: {
        header_value value{ m_data.value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    };

    case 5: {
        if (key != "$HANDSEED") {
            return parse_error();
        }

        header_value value{ m_data.value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 6: {
        if (key != "$CELTYPE" && key != "$DIMLTYPE" && key != "$DIMLTEX1" && key != "$DIMLTEX2") {
            return parse_error();
        }

        header_value value{ m_data.value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 7: {
        if (key != "$TEXTSTYLE" && key != "$DIMTXSTY") {
            return parse_error();
        }

        header_value value{ m_data.value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 8: {
        if (key != "$CLAYER") {
            return parse_error();
        }

        header_value value{ m_data.value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 10: {
        std::expected<std::variant<coordinate2d, coordinate3d>, error> maybe_coordinate{
            read_header_coordinate()
        };

        if (!maybe_coordinate) {
            return std::unexpected(maybe_coordinate.error());
        }

        header_value value{ std::visit(
            [](const auto& coordinate) -> header_value { return coordinate; },
            maybe_coordinate.value()) };

        return std::pair{ std::move(key), value };
    }

    case 40: {
        const std::optional<double> maybe_value{ parse_as<double>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 50: {
        if (key != "$ANGBASE") {
            return parse_error();
        }
        const std::optional<double> maybe_value{ parse_as<double>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 62: {
        if (key != "$CECOLOR" && key != "$INTERFERECOLOR") {
            return parse_error();
        }

        const std::optional<int> maybe_value{ parse_as<int>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 70: {
        const std::optional<int> maybe_value{ parse_as<int>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value };

        if (!read_next()) {
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
            return parse_error();
        }

        const std::optional<int> maybe_value{ parse_as<int>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 290: {
        const std::optional<int> maybe_value{ parse_as<int>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value != 0 };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 370: {
        if (key != "$CELWEIGHT") {
            return parse_error();
        }

        const std::optional<int> maybe_value{ parse_as<int>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    case 380: {
        if (key != "$CEPSNTYPE") {
            return parse_error();
        }

        const std::optional<int> maybe_value{ parse_as<int>(m_data.value) };
        if (!maybe_value) {
            return parse_error();
        }

        header_value value{ *maybe_value };

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }

        return std::pair{ std::move(key), std::move(value) };
    }

    default: {
        return std::unexpected(
            error{
                .line_number = m_current_line - 1,
                .what = std::format(
                    "unexpected group code {} for header variable name", m_data.group_code),
            });
    }
    }

    return {};
}

std::expected<std::variant<coordinate2d, coordinate3d>, error> reader::read_header_coordinate()
{
    double x{ 0.0 };
    double y{ 0.0 };
    std::optional<double> z;

    while (m_data.group_code == 10 || m_data.group_code == 20 || m_data.group_code == 30) {
        switch (m_data.group_code) {
        case 10: {
            if (!parse_into(x)) {
                return std::unexpected(m_error.value());
            }

            break;
        }

        case 20: {
            if (!parse_into(y)) {
                return std::unexpected(m_error.value());
            }

            break;
        }

        case 30: {
            if (!parse_into(z)) {
                return std::unexpected(m_error.value());
            }

            break;
        }
        }

        if (!read_next()) {
            return std::unexpected(m_error.value());
        }
    }

    if (z.has_value()) {
        return coordinate3d{ x, y, *z };
    }

    return coordinate2d{ x, y };
}

std::expected<void, error> reader::read_tables()
{
    if (!read_next()) {
        return make_error();
    }

    if (!is_section_begin()) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected section begin",
            });
    }

    if (!read_next()) {
        return make_error();
    }

    if (!is_tables_begin()) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected tables begin",
            });
    }

    while (!is_section_end()) {
        if (has_error()) {
            return make_error();
        }

        if (is_layer()) {
            std::expected<void, error> maybe_result{ read_layer() };
            if (!maybe_result) {
                return maybe_result;
            }
        } else if (is_line_type()) {
            std::expected<void, error> maybe_result{ read_line_type() };
            if (!maybe_result) {
                return maybe_result;
            }
        } else {
            read_next();
        }

        if (has_error()) {
            return make_error();
        }
    }

    return {};
}

std::expected<void, error> reader::read_line_type()
{
    if (!read_next()) {
        return std::unexpected(m_error.value());
    }

    odxf::line_type line_type;
    while (m_data.group_code != 0) {
        switch (m_data.group_code) {
        case 2: {
            line_type.name = m_data.value;

            break;
        }

        case 3: {
            line_type.display_name = m_data.value;

            break;
        }

        case 70: {
            if (!parse_into(line_type.flags)) {
                return make_error();
            }

            break;
        }

        case 49: {
            const std::optional<double> maybe_length{ parse_as<double>(m_data.value) };
            if (maybe_length.has_value()) {
                line_type.elements.push_back(line_type_element{ .length = *maybe_length });
            } else {
                return parse_error();
            }

            break;
        }

        // Follows the 49 of its element.
        case 74: {
            const std::optional<int> maybe_flags{ parse_as<int>(m_data.value) };
            if (!maybe_flags.has_value() || line_type.elements.empty()) {
                return parse_error();
            }
            line_type.elements.back().complex_flags = *maybe_flags;

            break;
        }
        }

        if (!read_next()) {
            return make_error();
        }
    }

    m_stream.line_type(line_type);

    return std::expected<void, error>();
}

std::expected<void, error> reader::read_layer()
{
    if (!read_next()) {
        return std::unexpected(m_error.value());
    }

    odxf::layer layer;
    // The application that owns the xdata read next, empty before the first group code 1001.
    std::string xdata_app;
    while (m_data.group_code != 0) {
        switch (m_data.group_code) {
        case 2: {
            layer.name = m_data.value;

            break;
        }

        case 62: {
            if (!parse_into(layer.color)) {
                return make_error();
            }

            break;
        }

        case 420: {
            if (!parse_into(layer.true_color)) {
                return make_error();
            }

            break;
        }

        case 370: {
            if (!parse_into(layer.lineweight)) {
                return make_error();
            }

            break;
        }

        case 6: {
            layer.line_type = m_data.value;

            break;
        }

        case 70: {
            const std::optional<int> maybe_flags{ parse_as<int>(m_data.value) };
            if (maybe_flags.has_value()) {
                layer.flags = static_cast<odxf::layer::flag>(*maybe_flags);
            } else {
                return parse_error();
            }

            break;
        }

        case 1001: {
            xdata_app = m_data.value;

            break;
        }

        case 1071: {
            if (xdata_app != layer_transparency_app) {
                break;
            }

            const std::optional<int> maybe_transparency{ parse_as<int>(m_data.value) };
            if (maybe_transparency.has_value()) {
                layer.transparency = decode_layer_transparency(*maybe_transparency);
            } else {
                return parse_error();
            }

            break;
        }
        }

        if (!read_next()) {
            return make_error();
        }
    }

    m_stream.layer(layer);

    return std::expected<void, error>();
}

std::expected<void, error> reader::read_blocks()
{
    if (!read_next()) {
        return make_error();
    }

    if (!is_section_begin()) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected section begin",
            });
    }

    if (!read_next()) {
        return make_error();
    }

    if (!is_blocks_begin()) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected blocks begin",
            });
    }

    while (read_next() && !is_section_end()) {
        if (has_error()) {
            return make_error();
        }
    }

    return {};
}

std::expected<void, error> reader::read_entities()
{
    if (!read_next()) {
        return make_error();
    }

    if (!is_section_begin()) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected section begin",
            });
    }

    if (!read_next()) {
        return make_error();
    }

    if (!is_entities_begin()) {
        return std::unexpected(
            error{
                .line_number = m_current_line,
                .what = "expected entities begin",
            });
    }

    while (!is_section_end()) {
        if (has_error()) {
            return make_error();
        }

        if (is_line()) {
            std::expected<void, error> maybe_result{ read_line() };
            if (!maybe_result) {
                return maybe_result;
            }

        } else if (is_circle()) {
            std::expected<void, error> maybe_result{ read_circle() };
            if (!maybe_result) {
                return maybe_result;
            }
        } else if (is_arc()) {
            std::expected<void, error> maybe_result{ read_arc() };
            if (!maybe_result) {
                return maybe_result;
            }
        } else if (is_lw_polyline()) {
            std::expected<void, error> maybe_result{ read_lw_polyline() };
            if (!maybe_result) {
                return maybe_result;
            }
        } else {
            read_next();
        }

        if (has_error()) {
            return make_error();
        }
    }

    return {};
}

template <entity_type E, std::predicate<E&> F>
std::expected<E, error> reader::read_entity(F read_group_code)
{
    if (!read_next()) {
        return std::unexpected(m_error.value());
    }

    E entity;
    while (m_data.group_code != 0) {
        if (!read_group_code(entity) || !read_next()) {
            return std::unexpected(m_error.value());
        }
    }

    return entity;
}

bool reader::read_entity_property(entity_properties& properties)
{
    switch (m_data.group_code) {
    case 8: properties.layer = m_data.value; return true;

    case 62: return parse_into(properties.color);

    case 420: return parse_into(properties.true_color);

    case 370: return parse_into(properties.lineweight);

    case 6: properties.line_type = m_data.value; return true;

    case 48: return parse_into(properties.line_type_scale);

    case 440: {
        int transparency{ 0 };
        if (!parse_into(transparency)) {
            return false;
        }
        properties.transparency = decode_entity_transparency(transparency);
        return true;
    }

    case 60: {
        int visibility{ 0 };
        if (!parse_into(visibility)) {
            return false;
        }
        // 1 means invisible, 0 and any other value visible
        properties.is_visible = visibility != 1;
        return true;
    }

    default:
        // unknown group codes are skipped
        return true;
    }
}

std::expected<void, error> reader::read_line()
{
    return read_entity<line>([this](odxf::line& line) -> bool {
               switch (m_data.group_code) {
               case 10: return parse_into(line.start.x);
               case 20: return parse_into(line.start.y);
               case 30: return parse_into(line.start.z);
               case 11: return parse_into(line.end.x);
               case 21: return parse_into(line.end.y);
               case 31: return parse_into(line.end.z);
               case 39: return parse_into(line.thickness);
               case 210: return parse_into(extrusion(line).x);
               case 220: return parse_into(extrusion(line).y);
               case 230: return parse_into(extrusion(line).z);
               default: return read_entity_property(line.properties);
               }
           })
        .transform([this](const odxf::line& line) { m_stream.line(line); });
}

std::expected<void, error> reader::read_circle()
{
    return read_entity<circle>([this](odxf::circle& circle) -> bool {
               switch (m_data.group_code) {
               case 10: return parse_into(circle.center.x);
               case 20: return parse_into(circle.center.y);
               case 30: return parse_into(circle.center.z);
               case 40: return parse_into(circle.radius);
               case 39: return parse_into(circle.thickness);
               case 210: return parse_into(extrusion(circle).x);
               case 220: return parse_into(extrusion(circle).y);
               case 230: return parse_into(extrusion(circle).z);
               default: return read_entity_property(circle.properties);
               }
           })
        .transform([this](const odxf::circle& circle) { m_stream.circle(circle); });
}

std::expected<void, error> reader::read_arc()
{
    return read_entity<arc>([this](odxf::arc& arc) -> bool {
               switch (m_data.group_code) {
               case 10: return parse_into(arc.center.x);
               case 20: return parse_into(arc.center.y);
               case 30: return parse_into(arc.center.z);
               case 40: return parse_into(arc.radius);
               case 39: return parse_into(arc.thickness);
               case 210: return parse_into(extrusion(arc).x);
               case 220: return parse_into(extrusion(arc).y);
               case 230: return parse_into(extrusion(arc).z);
               case 50: return parse_into(arc.start_angle);
               case 51: return parse_into(arc.end_angle);
               default: return read_entity_property(arc.properties);
               }
           })
        .transform([this](const odxf::arc& arc) { m_stream.arc(arc); });
}

std::expected<void, error> reader::read_lw_polyline()
{
    odxf::vertex vertex;
    int num_xy{ 0 };

    // A vertex is complete once it has x and y. Its bulge, if any, follows directly.
    const auto push_complete_vertex = [&](odxf::lw_polyline& lw_polyline) {
        if (num_xy == 2) {
            lw_polyline.vertices.push_back(vertex);
            num_xy = 0;
            vertex.bulge.reset();
        }
    };

    std::expected<lw_polyline, error> maybe_lw_polyline{ read_entity<lw_polyline>(
        [&](odxf::lw_polyline& lw_polyline) -> bool {
            switch (m_data.group_code) {
            case 90: {
                int vertices_count{ 0 };
                if (!parse_into(vertices_count)) {
                    return false;
                }
                if (vertices_count >= 0) {
                    lw_polyline.vertices.reserve(static_cast<std::size_t>(vertices_count));
                }
                return true;
            }

            case 70: {
                int flags{ 0 };
                if (!parse_into(flags)) {
                    return false;
                }
                lw_polyline.is_closed = (flags & 1) != 0;
                return true;
            }

            case 10:
                push_complete_vertex(lw_polyline);
                if (!parse_into(vertex.position.x)) {
                    return false;
                }
                num_xy++;
                return true;

            case 20:
                push_complete_vertex(lw_polyline);
                if (!parse_into(vertex.position.y)) {
                    return false;
                }
                num_xy++;
                return true;

            case 42:
                if (!parse_into(vertex.bulge)) {
                    return false;
                }
                push_complete_vertex(lw_polyline);
                return true;

            default: return read_entity_property(lw_polyline.properties);
            }
        }) };

    if (!maybe_lw_polyline) {
        return std::unexpected(maybe_lw_polyline.error());
    }

    push_complete_vertex(*maybe_lw_polyline);
    m_stream.lw_polyline(*maybe_lw_polyline);

    return {};
}

bool reader::read_next()
{
    // Group code 999 is a comment, possibly several in a row. Comments are skipped.
    bool success{ false };
    do {
        success = read_next_single();
    } while (m_data.group_code == 999 && success);

    return success;
}

bool reader::read_next_single()
{
    if (!read_text_line()) {
        return false;
    }

    const std::optional<int> maybe_group_code{ parse_as<int>(m_data.value) };
    if (!maybe_group_code) {
        m_error = error{
            .line_number = m_current_line,
            .what = "unable to parse group code",
        };

        return false;
    }
    m_data.group_code = *maybe_group_code;

    return read_text_line();
}

bool reader::read_text_line()
{
    m_current_line++;

    if (!std::getline(m_file_stream, m_data.value)) {
        m_error = error{
            .line_number = m_current_line,
            .what = "unable to read line",
        };

        return false;
    }

    if (m_data.value.ends_with('\r')) {
        m_data.value.pop_back();
    }

    return true;
}

bool reader::has_error() const { return m_error.has_value(); }

std::expected<void, error> reader::make_error() const { return std::unexpected(m_error.value()); }

std::unexpected<error> reader::parse_error() const
{
    return std::unexpected(error{ .line_number = m_current_line });
}

bool reader::is_section_begin() const
{
    return m_data.group_code == 0 && m_data.value == "SECTION";
}

bool reader::is_section_end() const { return m_data.group_code == 0 && m_data.value == "ENDSEC"; }

bool reader::is_header_begin() const { return m_data.group_code == 2 && m_data.value == "HEADER"; }

bool reader::is_tables_begin() const { return m_data.group_code == 2 && m_data.value == "TABLES"; }

bool reader::is_blocks_begin() const { return m_data.group_code == 2 && m_data.value == "BLOCKS"; }

bool reader::is_entities_begin() const
{
    return m_data.group_code == 2 && m_data.value == "ENTITIES";
}

bool reader::is_table_begin() const { return m_data.group_code == 0 && m_data.value == "TABLE"; }

bool reader::is_table_end() const { return m_data.group_code == 0 && m_data.value == "ENDTAB"; }

bool reader::is_layer_begin() const { return m_data.group_code == 2 && m_data.value == "LAYER"; }

bool reader::is_layer() const { return m_data.group_code == 0 && m_data.value == "LAYER"; }

bool reader::is_line_type() const { return m_data.group_code == 0 && m_data.value == "LTYPE"; }

bool reader::is_line() const { return m_data.group_code == 0 && m_data.value == "LINE"; }

bool odxf::reader::is_circle() const { return m_data.group_code == 0 && m_data.value == "CIRCLE"; }

bool reader::is_arc() const { return m_data.group_code == 0 && m_data.value == "ARC"; }

bool reader::is_lw_polyline() const
{
    return m_data.group_code == 0 && m_data.value == "LWPOLYLINE";
}

bool reader::is_eof() const { return m_data.group_code == 0 && m_data.value == "EOF"; }

}   // namespace odxf
