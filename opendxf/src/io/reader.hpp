// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/coordinate.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/io/error.hpp"

#include <concepts>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace odxf {

class read_stream_base;

// A type that holds the properties all entities share, e.g. Line.
template <typename E>
concept entity_type = requires(E& entity) {
    { entity.properties } -> std::same_as<entity_properties&>;
};

class reader
{
public:
    reader(read_stream_base& stream);

    reader(const reader&) = delete;
    reader(reader&&) = delete;
    reader& operator=(const reader&) = delete;
    reader& operator=(reader&&) = delete;

    std::expected<void, error> read_all(const std::filesystem::path& file_path);

private:
    bool open(const std::filesystem::path& file_path);

    std::expected<void, error> read_header();
    std::expected<header_entry, error> read_header_entry();
    std::expected<std::variant<coordinate2d, coordinate3d>, error> read_header_coordinate();

    std::expected<void, error> read_tables();
    std::expected<void, error> read_line_type();
    std::expected<void, error> read_layer();

    std::expected<void, error> read_blocks();

    std::expected<void, error> read_entities();
    std::expected<void, error> read_line();
    std::expected<void, error> read_circle();
    std::expected<void, error> read_arc();
    std::expected<void, error> read_lw_polyline();

    // Reads the group codes of one entity up to the next group code 0. read_group_code is called
    // for each group code and returns false on an error.
    template <entity_type E, std::predicate<E&> F>
    std::expected<E, error> read_entity(F read_group_code);
    // Reads the group codes all entities share, skips unknown ones. Returns false on an error.
    bool read_entity_property(entity_properties& properties);

    bool read_next();
    bool read_next_single();
    bool read_text_line();

    // parse the current value into target, on failure sets the error and returns false
    template <typename T>
    bool parse_into(T& target);
    template <typename T>
    bool parse_into(std::optional<T>& target);

    bool has_error() const;
    std::expected<void, error> make_error() const;
    // an Error at the current line, without a description
    std::unexpected<error> parse_error() const;
    bool is_section_begin() const;
    bool is_section_end() const;
    bool is_header_begin() const;
    bool is_tables_begin() const;
    bool is_blocks_begin() const;
    bool is_entities_begin() const;
    bool is_table_begin() const;
    bool is_table_end() const;
    bool is_layer_begin() const;
    bool is_layer() const;
    bool is_line_type() const;

    bool is_line() const;
    bool is_circle() const;
    bool is_arc() const;
    bool is_lw_polyline() const;

    bool is_eof() const;

    struct data
    {
        int group_code;
        std::string value;
    };

    read_stream_base& m_stream;
    std::ifstream m_file_stream;
    data m_data;
    int m_current_line{ 0 };
    std::optional<error> m_error;
    std::vector<std::string> m_layer_names;
};

}   // namespace odxf
