// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/opendxf.hpp"

#include <fmt/core.h>
#include <tl/expected.hpp>

#include <filesystem>
#include <iostream>
#include <string>

struct DocumentStats final
{
    int layers{ 0 };

    int arcs{ 0 };
    int circles{ 0 };
    int lines{ 0 };
    int lwPolylines{ 0 };
};

void printDocumentStats(const DocumentStats& stats)
{
    fmt::print(
        "layers: {}\narcs: {}\ncircles: {}\nlines: {}\nlw polylines: {}\n",
        stats.layers,
        stats.arcs,
        stats.circles,
        stats.lines,
        stats.lwPolylines);
}

void printError(const odxf::Error& error)
{
    if (error.lineNumber.has_value()) {
        fmt::print("error reading file: line {}: {}\n", error.lineNumber.value(), error.what);
    } else {
        fmt::print("error reading file: {}\n", error.what);
    }
}

tl::expected<std::filesystem::path, std::string> parseFilePath(int argc, char** argv)
{
    if (argc != 2) {
        return tl::make_unexpected("invalid number of arguments");
    }

    return reinterpret_cast<const char8_t*>(argv[1]);
}

void printUsage(const std::string& error) { fmt::print("error: {}\n", error); }

class Reader final : public odxf::IReadStream
{
public:
    const DocumentStats& stats() const& { return m_docStats; }

private:
    void layer(const odxf::Layer& /* layer */) override { m_docStats.layers += 1; }

    void arc(const odxf::Arc& /* arc */) override { m_docStats.arcs += 1; }

    void circle(const odxf::Circle& /* circle */) override { m_docStats.circles += 1; }

    void line(const odxf::Line& /* line */) override { m_docStats.lines += 1; }

    void lwPolyline(const odxf::LWPolyline& /* lwPolyline */) override
    {
        m_docStats.lwPolylines += 1;
    }

    DocumentStats m_docStats;
};

int main(int argc, char** argv)
{
    Reader reader;

    const tl::expected<std::filesystem::path, std::string> filePath{ parseFilePath(argc, argv) };
    if (!filePath.has_value()) {
        printUsage(filePath.error());

        return EXIT_FAILURE;
    }

    odxf::read(reader, filePath.value())
        .transform([&reader] { printDocumentStats(reader.stats()); })
        .or_else(printError);

    return EXIT_SUCCESS;
}
