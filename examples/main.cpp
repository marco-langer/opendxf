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
        .entities{ .arcs{ { .center{ 0.0F, 2.0F },
                            .radius{ 0.5F },
                            .startAngle{ 0.0F },
                            .endAngle{ 180.0F } } },
                   .circles{ { .center{}, .radius{ 1.0F } } },
                   .lines{
                       { .start{ 0.0F, 0.5F, 0.0F }, .end{ 1.0F, 1.5F, 0.0F } },
                       { .end{ 1.0F, 1.0F, 0.0F } },
                   } },
    };
    odxf::writeDxf(document, "test.dxf");

    return EXIT_SUCCESS;
}
