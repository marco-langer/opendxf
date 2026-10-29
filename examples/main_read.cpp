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
    int lwPolylines{ 0 };
};

void printDocumentStats(const document_stats& stats)
{
    std::println(
        "layers: {}\narcs: {}\ncircles: {}\nlines: {}\nlw polylines: {}",
        stats.layers,
        stats.arcs,
        stats.circles,
        stats.lines,
        stats.lwPolylines);
}

void printError(const odxf::error& error)
{
    if (error.lineNumber.has_value()) {
        std::println("error reading file: line {}: {}", error.lineNumber.value(), error.what);
    } else {
        std::println("error reading file: {}", error.what);
    }
}

std::expected<std::filesystem::path, std::string> parseFilePath(int argc, char** argv)
{
    if (argc != 2) {
        return std::unexpected("invalid number of arguments");
    }

    return reinterpret_cast<const char8_t*>(argv[1]);
}

void printUsage(const std::string& error) { std::println("error: {}", error); }

class reader final : public odxf::read_stream_base
{
public:
    const document_stats& stats() const& { return m_docStats; }

private:
    void layer(const odxf::layer& /* layer */) override { m_docStats.layers += 1; }

    void arc(const odxf::arc& /* arc */) override { m_docStats.arcs += 1; }

    void circle(const odxf::circle& /* circle */) override { m_docStats.circles += 1; }

    void line(const odxf::line& /* line */) override { m_docStats.lines += 1; }

    void lwPolyline(const odxf::lw_polyline& /* lwPolyline */) override
    {
        m_docStats.lwPolylines += 1;
    }

    document_stats m_docStats;
};

int main(int argc, char** argv)
{
    reader reader;

    const std::expected<std::filesystem::path, std::string> filePath{ parseFilePath(argc, argv) };
    if (!filePath.has_value()) {
        printUsage(filePath.error());

        return EXIT_FAILURE;
    }

    const std::expected<void, odxf::error> result{ odxf::read(filePath.value(), reader) };
    if (!result.has_value()) {
        printError(result.error());

        return EXIT_FAILURE;
    }

    printDocumentStats(reader.stats());

    return EXIT_SUCCESS;
}
