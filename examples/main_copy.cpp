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
class LayerFilter final : public odxf::IReadStream
{
public:
    LayerFilter(std::filesystem::path outputPath, std::string layer)
        : m_outputPath{ std::move(outputPath) }
        , m_layer{ std::move(layer) }
    {
    }

    // Call after reading. Opens the output if no entity did, then closes it.
    std::expected<void, odxf::Error> close()
    {
        if (!m_error && open()) {
            if (std::expected<void, odxf::Error> maybeError = m_stream->close(); !maybeError) {
                m_error = maybeError.error();
            }
        }

        return m_error ? std::unexpected(*m_error) : std::expected<void, odxf::Error>{};
    }

private:
    // Header and tables arrive before the first entity.
    void header(const odxf::Header& header) override { m_header = header; }
    void lineType(const odxf::LineType& lineType) override
    {
        m_tables.lineTypes.push_back(lineType);
    }
    void layer(const odxf::Layer& layer) override { m_tables.layers.push_back(layer); }

    void arc(const odxf::Arc& arc) override { copy(arc); }
    void circle(const odxf::Circle& circle) override { copy(circle); }
    void line(const odxf::Line& line) override { copy(line); }
    void lwPolyline(const odxf::LWPolyline& lwPolyline) override { copy(lwPolyline); }

    template <typename E>
    void copy(const E& entity)
    {
        if (m_error || entity.properties.layer != m_layer || !open()) {
            return;
        }

        // The hooks can't stop the reader, so the first error is kept and reported by close().
        if (std::expected<void, odxf::Error> maybeError = m_stream->entity(entity); !maybeError) {
            m_error = maybeError.error();
        }
    }

    bool open()
    {
        if (m_stream) {
            return true;
        }

        std::expected<odxf::WriteStream, odxf::Error> maybeStream{ odxf::WriteStream::open(
            m_outputPath, m_header, m_tables) };
        if (!maybeStream) {
            m_error = maybeStream.error();
            return false;
        }

        m_stream.emplace(std::move(*maybeStream));
        return true;
    }

    std::filesystem::path m_outputPath;
    std::string m_layer;

    odxf::Header m_header;
    odxf::Tables m_tables;
    std::optional<odxf::WriteStream> m_stream;
    std::optional<odxf::Error> m_error;
};

void printError(const std::string& context, const odxf::Error& error)
{
    if (error.lineNumber.has_value()) {
        std::println("{}: line {}: {}", context, error.lineNumber.value(), error.what);
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

    const std::filesystem::path inputPath{ reinterpret_cast<const char8_t*>(argv[1]) };
    const std::filesystem::path outputPath{ reinterpret_cast<const char8_t*>(argv[2]) };

    LayerFilter filter{ outputPath, argv[3] };

    if (std::expected<void, odxf::Error> result = odxf::read(inputPath, filter); !result) {
        printError("error reading file", result.error());

        return EXIT_FAILURE;
    }

    if (std::expected<void, odxf::Error> result = filter.close(); !result) {
        printError("error writing file", result.error());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
