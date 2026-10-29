// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/opendxf.hpp"

#include <print>

int main()
{
    odxf::Document document{
        .header{ .entries{ { "$ACADVER", "AC1032" } } },
        .tables{
            .lineTypes{
                { .name{ "CONTINUOUS" }, .displayName{ "Solid Line" } },
            },
            .layers{ { .name{ "Test Layer" } } },
        },
        .entities{
            odxf::Arc{
                .center{ 0.0, 2.0 }, .radius{ 0.5 }, .startAngle{ 0.0 }, .endAngle{ 180.0 } },
            odxf::Circle{ .center{}, .radius{ 1.0 } },
            odxf::Line{ .start{ 0.0, 0.5, 0.0 }, .end{ 1.0, 1.5, 0.0 } },
            odxf::Line{ .end{ 1.0, 1.0, 0.0 } },
        },
    };

    const std::expected<void, odxf::Error> result{ odxf::write("test.dxf", document) };
    if (!result) {
        std::println("error: {}", result.error().what);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
