// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "opendxf/opendxf.hpp"

#include <expected>
#include <filesystem>
#include <optional>
#include <print>
#include <string>
#include <utility>

// Copies a file entity by entity and keeps only the entities on one layer. Header and tables are
// kept as they are. No more than one entity is held in memory.
class layer_filter final : public odxf::read_stream_base
{
public:
    layer_filter(std::filesystem::path output_path, std::string layer)
        : m_output_path{ std::move(output_path) }
        , m_layer{ std::move(layer) }
    {
    }

    // Call after reading. Opens the output if no entity did, then closes it.
    std::expected<void, odxf::error> close()
    {
        if (!m_error && open()) {
            if (std::expected<void, odxf::error> maybe_error = m_stream->close(); !maybe_error) {
                m_error = maybe_error.error();
            }
        }

        return m_error ? std::unexpected(*m_error) : std::expected<void, odxf::error>{};
    }

private:
    // Header and tables arrive before the first entity.
    void header(const odxf::header& header) override { m_header = header; }
    void line_type(const odxf::line_type& line_type) override
    {
        m_tables.line_types.push_back(line_type);
    }
    void layer(const odxf::layer& layer) override { m_tables.layers.push_back(layer); }

    void arc(const odxf::arc& arc) override { copy(arc); }
    void circle(const odxf::circle& circle) override { copy(circle); }
    void line(const odxf::line& line) override { copy(line); }
    void lw_polyline(const odxf::lw_polyline& lw_polyline) override { copy(lw_polyline); }

    template <typename E>
    void copy(const E& entity)
    {
        if (m_error || entity.properties.layer != m_layer || !open()) {
            return;
        }

        // The hooks can't stop the reader, so the first error is kept and reported by close().
        if (std::expected<void, odxf::error> maybe_error = m_stream->entity(entity); !maybe_error) {
            m_error = maybe_error.error();
        }
    }

    bool open()
    {
        if (m_stream) {
            return true;
        }

        std::expected<odxf::write_stream, odxf::error> maybe_stream{ odxf::write_stream::open(
            m_output_path, m_header, m_tables) };
        if (!maybe_stream) {
            m_error = maybe_stream.error();
            return false;
        }

        m_stream.emplace(std::move(*maybe_stream));
        return true;
    }

    std::filesystem::path m_output_path;
    std::string m_layer;

    odxf::header m_header;
    odxf::tables m_tables;
    std::optional<odxf::write_stream> m_stream;
    std::optional<odxf::error> m_error;
};

void print_error(const std::string& context, const odxf::error& error)
{
    if (error.line_number.has_value()) {
        std::println("{}: line {}: {}", context, error.line_number.value(), error.what);
    } else {
        std::println("{}: {}", context, error.what);
    }
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::println("usage: opendxf_example_copy <input.dxf> <output.dxf> <layer>");

        return EXIT_FAILURE;
    }

    const std::filesystem::path input_path{ reinterpret_cast<const char8_t*>(argv[1]) };
    const std::filesystem::path output_path{ reinterpret_cast<const char8_t*>(argv[2]) };

    layer_filter filter{ output_path, argv[3] };

    if (std::expected<void, odxf::error> result = odxf::read(input_path, filter); !result) {
        print_error("error reading file", result.error());

        return EXIT_FAILURE;
    }

    if (std::expected<void, odxf::error> result = filter.close(); !result) {
        print_error("error writing file", result.error());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
