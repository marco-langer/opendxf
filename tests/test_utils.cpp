// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "test_utils.hpp"

#include "opendxf/coordinate.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/layer.hpp"

odxf::Document createExampleDocument()
{
    using namespace odxf;

    Document expectedDocument;
    auto& headerEntries{ expectedDocument.header.entries };
    headerEntries.try_emplace("$TEXTSIZE", 2.5);
    headerEntries.try_emplace("$LTSCALE", 2.0);
    headerEntries.try_emplace("$REGENMODE", 1);
    headerEntries.try_emplace("$ORTHOMODE", 0);
    headerEntries.try_emplace("$LIMMAX", odxf::Coordinate2d{ 420, 297 });
    headerEntries.try_emplace("$LIMMIN", odxf::Coordinate2d{});
    headerEntries.try_emplace(
        "$EXTMIN", odxf::Coordinate3d{ 77.48842691804612, 27.48842691804612, 0.0 });
    headerEntries.try_emplace("$INSBASE", odxf::Coordinate3d{});
    headerEntries.try_emplace("$ACADVER", "AC1032");
    headerEntries.try_emplace("$TEXTSTYLE", "STANDARD");

    const std::string layerNameTest{ "Test Layer" };

    headerEntries.try_emplace("$CLAYER", layerNameTest);
    headerEntries.try_emplace("$CELTYPE", "BYLAYER");
    headerEntries.try_emplace("$CECOLOR", 200);
    headerEntries.try_emplace("$DIMSTYLE", "STANDARD");
    headerEntries.try_emplace("$DIMTXSTY", "STANDARD");
    headerEntries.try_emplace("$DIMLTYPE", std::string{});
    headerEntries.try_emplace("$DIMLTEX1", std::string{});
    headerEntries.try_emplace("$DIMLTEX2", std::string{});
    headerEntries.try_emplace("$ANGBASE", 0.0);
    headerEntries.try_emplace("$HANDSEED", "20000");
    headerEntries.try_emplace("$CELWEIGHT", -1);
    headerEntries.try_emplace("$ENDCAPS", 1);
    headerEntries.try_emplace("$JOINSTYLE", 2);
    headerEntries.try_emplace("$LWDISPLAY", false);
    headerEntries.try_emplace("$INSUNITS", 4);
    headerEntries.try_emplace("$XEDIT", true);
    headerEntries.try_emplace("$CEPSNTYPE", 1);
    headerEntries.try_emplace("$SORTENTS", 127);
    headerEntries.try_emplace("$INDEXCTL", 0);
    headerEntries.try_emplace("$HIDETEXT", 1);
    headerEntries.try_emplace("$HALOGAP", 0);
    headerEntries.try_emplace("$OBSLTYPE", 0);
    headerEntries.try_emplace("$INTERSECTIONDISPLAY", 1);
    headerEntries.try_emplace("$DIMASSOC", 1);
    headerEntries.try_emplace("$LOFTNORMALS", 1);
    headerEntries.try_emplace("$LIGHTGLYPHDISPLAY", 1);
    headerEntries.try_emplace("$TILEMODELIGHTSYNCH", 1);
    headerEntries.try_emplace("$SOLIDHIST", 1);
    headerEntries.try_emplace("$SHOWHIST", 1);
    headerEntries.try_emplace("$DWFFRAME", 2);
    headerEntries.try_emplace("$DGNFRAME", 0);
    headerEntries.try_emplace("$CSHADOW", 0);
    headerEntries.try_emplace("$INTERFERECOLOR", 1);

    const std::string layerNameLines{ "Lines" };
    const std::string layerNameLWPolylines{ "LW Polylines" };
    const std::string layerNameTrueColor{ "True Color" };

    LineTypes& lineTypes{ expectedDocument.tables.lineTypes };
    lineTypes.push_back(
        LineType{
            .name = "CONTINUOUS",
            .displayName = "Solid Line",
        });
    lineTypes.push_back(
        LineType{
            .name = "DASHED",
            .displayName = "Dashed __ __ __",
            .flags = 0,
            .elements = { { .length = 0.5 }, { .length = -0.25 } },
        });
    lineTypes.push_back(
        LineType{
            .name = "DOT_DASH",
            .displayName = "Dot dash _ . _ .",
            .flags = 0,
            .elements = { { .length = 0.5 },
                          { .length = -0.25 },
                          { .length = 0.0 },
                          { .length = -0.25 } },
        });
    // The text element's details (75, 340, 46, 50, 44, 45, 9) are skipped.
    lineTypes.push_back(
        LineType{
            .name = "GAS",
            .displayName = "Gas line ---- GAS ----",
            .flags = 0,
            .elements = { { .length = 0.5 },
                          { .length = -0.2, .complexFlags = 2 },
                          { .length = -0.25 } },
        });

    Layers& layers{ expectedDocument.tables.layers };
    // layerNameTest has no group code 370 and keeps the default lineweight
    layers.emplace_back(Layer{ .name = layerNameTest }).flags = Layer::Flags::Frozen;
    layers.push_back(
        Layer{ .name = layerNameLines, .color = 1, .lineweight = 50, .lineType = "DASHED" });
    layers.push_back(Layer{ .name = layerNameLWPolylines, .color = -3, .lineweight = -3 });
    layers.push_back(
        Layer{ .name = layerNameTrueColor, .color = 5, .trueColor = 0x3050A0, .lineweight = 0 });

    // layers[0] has a BYBLOCK transparency, which layers can't have, and stays opaque. layers[3]
    // only has xdata of another application with a 1071.
    layers[1].transparency = Transparency{ .alpha = 0x4C };
    layers[2].transparency = Transparency{ .alpha = 0xFF };

    Entities& entities{ expectedDocument.entities };

    // no group codes 62, 370, 6, 48 and 440, keeps the defaults
    entities.push_back(
        Line{
            .properties = { .layer = layerNameLines, .trueColor = 0xFF8000 },
            .start = Coordinate3d{ 0.0, 0.5, 0.0 },
            .end = Coordinate3d{ 1.0, 1.5, 0.0 },
        });
    entities.push_back(Line{
        .properties = {
            .layer = layerNameLines,
            .color = 3,
            .lineweight = 35,
            .lineType = "DOT_DASH",
            .lineTypeScale = 2.5,
            .transparency = Transparency{ .alpha = 0x7F },
        },
        .start = Coordinate3d{ 0.0, 0.0, 0.0 },
        .end = Coordinate3d{ 1.0, 1.0, 0.0 },
    });

    entities.push_back(Circle{
        .properties = {
            .layer = layerNameTest,
            .color = 2,
            .trueColor = 0x00FF00,
            .lineweight = -2,
            .lineType = "BYBLOCK",
            .transparency = ByBlock{},
        },
        .center = Coordinate3d{ 0.0, 0.0, 0.0 },
        .radius = 1.0,
    });

    entities.push_back(Arc{
        .properties = {
            .layer = layerNameTest,
            .color = 5,
            .trueColor = 0x0000FF,
            .lineweight = -3,
            .lineType = "CONTINUOUS",
            .lineTypeScale = 0.5,
            .transparency = Transparency{ .alpha = 0x00 },
        },
        .center = Coordinate3d{ 0.0, 2.0, 0.0 },
        .radius = 0.5,
        .startAngle = 0.0,
        .endAngle = 180.0,
    });

    entities.push_back(LWPolyline{
        .properties = {
            .layer = layerNameLWPolylines,
            .color = 6,
            .trueColor = 0x808080,
            .lineweight = 211,
            .lineType = "GAS",
            .transparency = Transparency{ .alpha = 0xFF },
        },
    });
    // has a 440 without the flag of a set value, which counts as BYLAYER
    entities.push_back(LWPolyline{
        .properties = {
            .layer = layerNameLWPolylines,
            .color = 256,
            .lineweight = -1,
            .lineType = "BYLAYER",
        },
        .vertices = { Vertex{ .position = { 200.0, 200.0 } } },
    });
    // no group codes 6, 48 and 440, keeps the defaults
    entities.push_back(LWPolyline{
        .properties = { .layer = layerNameLWPolylines, .color = 0, .lineweight = 13 },
        .isClosed = true,
        .vertices = {
            Vertex{
                .position = { 10.0, 10.0 },
            },
            Vertex{
                .position = { 10.0, 50.0 },
                .bulge = 1.0,
            },
            Vertex{
                .position = { 20.0, 60.0, },
            },
            Vertex{
                .position = { 60.0, 60.0, },
                .bulge = -1.0
            },
            Vertex{
                .position = { 70.0, 50.0, },
            },
            Vertex{
                .position = { 70.0, 20.0 },
                .bulge = 1.0,
            },
            Vertex{
                .position = { 60.0, 10.0 },
            },
        },
    });

    return expectedDocument;
}
