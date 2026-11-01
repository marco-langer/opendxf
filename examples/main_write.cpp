// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include <opendxf/opendxf.hpp>

#include <print>

int main()
{
    odxf::document document{
        .header{ .entries{ { "$ACADVER", "AC1032" } } },
        .tables{
            .line_types{
                { .name{ "CONTINUOUS" }, .display_name{ "Solid Line" } },
            },
            .layers{ { .name{ "Test Layer" } } },
        },
        .entities{
            odxf::arc{ .properties{},
                       .center{ 0.0, 2.0 },
                       .radius{ 0.5 },
                       .start_angle{ 0.0 },
                       .end_angle{ 180.0 } },
            odxf::circle{ .properties{}, .center{}, .radius{ 1.0 } },
            odxf::line{ .properties{}, .start{ 0.0, 0.5, 0.0 }, .end{ 1.0, 1.5, 0.0 } },
            odxf::line{ .properties{}, .start{}, .end{ 1.0, 1.0, 0.0 } },
        },
    };

    const std::expected<void, odxf::error> result{ odxf::write("test.dxf", document) };
    if (!result) {
        std::println("error: {}", result.error().what);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
