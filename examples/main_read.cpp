// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/opendxf.hpp"

#include <expected>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>

struct document_stats final
{
    int layers{ 0 };

    int arcs{ 0 };
    int circles{ 0 };
    int lines{ 0 };
    int lw_polylines{ 0 };
};

void print_document_stats(const document_stats& stats)
{
    std::println(
        "layers: {}\narcs: {}\ncircles: {}\nlines: {}\nlw polylines: {}",
        stats.layers,
        stats.arcs,
        stats.circles,
        stats.lines,
        stats.lw_polylines);
}

void print_error(const odxf::error& error)
{
    if (error.line_number.has_value()) {
        std::println("error reading file: line {}: {}", error.line_number.value(), error.what);
    } else {
        std::println("error reading file: {}", error.what);
    }
}

std::expected<std::filesystem::path, std::string> parse_file_path(int argc, char** argv)
{
    if (argc != 2) {
        return std::unexpected("invalid number of arguments");
    }

    return reinterpret_cast<const char8_t*>(argv[1]);
}

void print_usage(const std::string& error) { std::println("error: {}", error); }

class reader final : public odxf::read_stream_base
{
public:
    const document_stats& stats() const& { return m_doc_stats; }

private:
    void layer(const odxf::layer& /* layer */) override { m_doc_stats.layers += 1; }

    void arc(const odxf::arc& /* arc */) override { m_doc_stats.arcs += 1; }

    void circle(const odxf::circle& /* circle */) override { m_doc_stats.circles += 1; }

    void line(const odxf::line& /* line */) override { m_doc_stats.lines += 1; }

    void lw_polyline(const odxf::lw_polyline& /* lw_polyline */) override
    {
        m_doc_stats.lw_polylines += 1;
    }

    document_stats m_doc_stats;
};

int main(int argc, char** argv)
{
    reader reader;

    const std::expected<std::filesystem::path, std::string> file_path{ parse_file_path(
        argc, argv) };
    if (!file_path.has_value()) {
        print_usage(file_path.error());

        return EXIT_FAILURE;
    }

    const std::expected<void, odxf::error> result{ odxf::read(file_path.value(), reader) };
    if (!result.has_value()) {
        print_error(result.error());

        return EXIT_FAILURE;
    }

    print_document_stats(reader.stats());

    return EXIT_SUCCESS;
}
