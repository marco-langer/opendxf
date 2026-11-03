// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include <opendxf/opendxf.hpp>

#include <expected>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>

namespace {

struct drawing_stats final
{
    int layers{ 0 };

    int arcs{ 0 };
    int circles{ 0 };
    int ellipses{ 0 };
    int lines{ 0 };
    int lw_polylines{ 0 };
    int points{ 0 };
    int rays{ 0 };
};

void print_drawing_stats(const drawing_stats& stats)
{
    std::println(
        "layers: {}\narcs: {}\ncircles: {}\nellipses: {}\nlines: {}\nlw polylines: {}\npoints: "
        "{}\nrays: {}",
        stats.layers,
        stats.arcs,
        stats.circles,
        stats.ellipses,
        stats.lines,
        stats.lw_polylines,
        stats.points,
        stats.rays);
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
    const drawing_stats& stats() const& { return m_drawing_stats; }

private:
    void layer(const odxf::layer& /* layer */) override { m_drawing_stats.layers += 1; }

    void arc(const odxf::arc& /* arc */) override { m_drawing_stats.arcs += 1; }

    void circle(const odxf::circle& /* circle */) override { m_drawing_stats.circles += 1; }

    void ellipse(const odxf::ellipse& /* ellipse */) override { m_drawing_stats.ellipses += 1; }

    void line(const odxf::line& /* line */) override { m_drawing_stats.lines += 1; }

    void lw_polyline(const odxf::lw_polyline& /* lw_polyline */) override
    {
        m_drawing_stats.lw_polylines += 1;
    }

    void point(const odxf::point& /* point */) override { m_drawing_stats.points += 1; }

    void ray(const odxf::ray& /* ray */) override { m_drawing_stats.rays += 1; }

    drawing_stats m_drawing_stats;
};

}   // namespace

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

    print_drawing_stats(reader.stats());

    return EXIT_SUCCESS;
}
