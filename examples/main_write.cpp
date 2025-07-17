// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/opendxf.hpp"

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
            .arcs{
                { .center{ 0.0, 2.0 }, .radius{ 0.5 }, .startAngle{ 0.0 }, .endAngle{ 180.0 } } },
            .circles{ { .center{}, .radius{ 1.0 } } },
            .lines{
                { .start{ 0.0, 0.5, 0.0 }, .end{ 1.0, 1.5, 0.0 } },
                { .end{ 1.0, 1.0, 0.0 } },
            } },
    };
    odxf::writeDxf(document, "test.dxf");

    return EXIT_SUCCESS;
}
