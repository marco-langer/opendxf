// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "test_utils.hpp"

#include "opendxf/coordinate.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/layer.hpp"

odxf::document create_example_document()
{
    using namespace odxf;

    document expected_document;
    auto& header_entries{ expected_document.header.entries };
    header_entries.try_emplace("$TEXTSIZE", 2.5);
    header_entries.try_emplace("$LTSCALE", 2.0);
    header_entries.try_emplace("$REGENMODE", 1);
    header_entries.try_emplace("$ORTHOMODE", 0);
    header_entries.try_emplace("$LIMMAX", odxf::coordinate2d{ 420, 297 });
    header_entries.try_emplace("$LIMMIN", odxf::coordinate2d{});
    header_entries.try_emplace(
        "$EXTMIN", odxf::coordinate3d{ 77.48842691804612, 27.48842691804612, 0.0 });
    header_entries.try_emplace("$INSBASE", odxf::coordinate3d{});
    header_entries.try_emplace("$ACADVER", "AC1032");
    header_entries.try_emplace("$TEXTSTYLE", "STANDARD");

    const std::string layer_name_test{ "Test Layer" };

    header_entries.try_emplace("$CLAYER", layer_name_test);
    header_entries.try_emplace("$CELTYPE", "BYLAYER");
    header_entries.try_emplace("$CECOLOR", 200);
    header_entries.try_emplace("$DIMSTYLE", "STANDARD");
    header_entries.try_emplace("$DIMTXSTY", "STANDARD");
    header_entries.try_emplace("$DIMLTYPE", std::string{});
    header_entries.try_emplace("$DIMLTEX1", std::string{});
    header_entries.try_emplace("$DIMLTEX2", std::string{});
    header_entries.try_emplace("$ANGBASE", 0.0);
    header_entries.try_emplace("$HANDSEED", "20000");
    header_entries.try_emplace("$CELWEIGHT", -1);
    header_entries.try_emplace("$ENDCAPS", 1);
    header_entries.try_emplace("$JOINSTYLE", 2);
    header_entries.try_emplace("$LWDISPLAY", false);
    header_entries.try_emplace("$INSUNITS", 4);
    header_entries.try_emplace("$XEDIT", true);
    header_entries.try_emplace("$CEPSNTYPE", 1);
    header_entries.try_emplace("$SORTENTS", 127);
    header_entries.try_emplace("$INDEXCTL", 0);
    header_entries.try_emplace("$HIDETEXT", 1);
    header_entries.try_emplace("$HALOGAP", 0);
    header_entries.try_emplace("$OBSLTYPE", 0);
    header_entries.try_emplace("$INTERSECTIONDISPLAY", 1);
    header_entries.try_emplace("$DIMASSOC", 1);
    header_entries.try_emplace("$LOFTNORMALS", 1);
    header_entries.try_emplace("$LIGHTGLYPHDISPLAY", 1);
    header_entries.try_emplace("$TILEMODELIGHTSYNCH", 1);
    header_entries.try_emplace("$SOLIDHIST", 1);
    header_entries.try_emplace("$SHOWHIST", 1);
    header_entries.try_emplace("$DWFFRAME", 2);
    header_entries.try_emplace("$DGNFRAME", 0);
    header_entries.try_emplace("$CSHADOW", 0);
    header_entries.try_emplace("$INTERFERECOLOR", 1);

    const std::string layer_name_lines{ "Lines" };
    const std::string layer_name_lw_polylines{ "LW Polylines" };
    const std::string layer_name_true_color{ "True Color" };

    odxf::line_types& line_types{ expected_document.tables.line_types };
    line_types.push_back(
        line_type{
            .name = "CONTINUOUS",
            .display_name = "Solid Line",
        });
    line_types.push_back(
        line_type{
            .name = "DASHED",
            .display_name = "Dashed __ __ __",
            .flags = 0,
            .elements = { { .length = 0.5 }, { .length = -0.25 } },
        });
    line_types.push_back(
        line_type{
            .name = "DOT_DASH",
            .display_name = "Dot dash _ . _ .",
            .flags = 0,
            .elements = { { .length = 0.5 },
                          { .length = -0.25 },
                          { .length = 0.0 },
                          { .length = -0.25 } },
        });
    // The text element's details (75, 340, 46, 50, 44, 45, 9) are skipped.
    line_types.push_back(
        line_type{
            .name = "GAS",
            .display_name = "Gas line ---- GAS ----",
            .flags = 0,
            .elements = { { .length = 0.5 },
                          { .length = -0.2, .complex_flags = 2 },
                          { .length = -0.25 } },
        });

    odxf::layers& layers{ expected_document.tables.layers };
    // layer_name_test has no group code 370 and keeps the default lineweight
    layers.emplace_back(layer{ .name = layer_name_test }).flags = layer::flag::frozen;
    layers.push_back(
        layer{ .name = layer_name_lines, .color = 1, .lineweight = 50, .line_type = "DASHED" });
    layers.push_back(layer{ .name = layer_name_lw_polylines, .color = -3, .lineweight = -3 });
    layers.push_back(
        layer{
            .name = layer_name_true_color, .color = 5, .true_color = 0x3050A0, .lineweight = 0 });

    // layers[0] has a BYBLOCK transparency, which layers can't have, and stays opaque. layers[3]
    // only has xdata of another application with a 1071.
    layers[1].transparency = transparency{ .alpha = 0x4C };
    layers[2].transparency = transparency{ .alpha = 0xFF };

    odxf::entities& entities{ expected_document.entities };

    // no group codes 62, 370, 6, 48 and 440, keeps the defaults
    entities.push_back(
        line{
            .properties = { .layer = layer_name_lines, .true_color = 0xFF8000 },
            .start = coordinate3d{ 0.0, 0.5, 0.0 },
            .end = coordinate3d{ 1.0, 1.5, 0.0 },
        });
    entities.push_back(line{
        .properties = {
            .layer = layer_name_lines,
            .color = 3,
            .lineweight = 35,
            .line_type = "DOT_DASH",
            .line_type_scale = 2.5,
            .transparency = transparency{ .alpha = 0x7F },
        },
        .start = coordinate3d{ 0.0, 0.0, 0.0 },
        .end = coordinate3d{ 1.0, 1.0, 0.0 },
    });

    entities.push_back(circle{
        .properties = {
            .layer = layer_name_test,
            .color = 2,
            .true_color = 0x00FF00,
            .lineweight = -2,
            .line_type = "BYBLOCK",
            .transparency = by_block{},
        },
        .center = coordinate3d{ 0.0, 0.0, 0.0 },
        .radius = 1.0,
    });

    entities.push_back(arc{
        .properties = {
            .layer = layer_name_test,
            .color = 5,
            .true_color = 0x0000FF,
            .lineweight = -3,
            .line_type = "CONTINUOUS",
            .line_type_scale = 0.5,
            .transparency = transparency{ .alpha = 0x00 },
        },
        .center = coordinate3d{ 0.0, 2.0, 0.0 },
        .radius = 0.5,
        .start_angle = 0.0,
        .end_angle = 180.0,
    });

    entities.push_back(lw_polyline{
        .properties = {
            .layer = layer_name_lw_polylines,
            .color = 6,
            .true_color = 0x808080,
            .lineweight = 211,
            .line_type = "GAS",
            .transparency = transparency{ .alpha = 0xFF },
        },
    });
    // has a 440 without the flag of a set value, which counts as BYLAYER
    entities.push_back(lw_polyline{
        .properties = {
            .layer = layer_name_lw_polylines,
            .color = 256,
            .lineweight = -1,
            .line_type = "BYLAYER",
        },
        .vertices = { vertex{ .position = { 200.0, 200.0 } } },
    });
    // no group codes 6, 48 and 440, keeps the defaults
    entities.push_back(lw_polyline{
        .properties = { .layer = layer_name_lw_polylines, .color = 0, .lineweight = 13 },
        .is_closed = true,
        .vertices = {
            vertex{
                .position = { 10.0, 10.0 },
            },
            vertex{
                .position = { 10.0, 50.0 },
                .bulge = 1.0,
            },
            vertex{
                .position = { 20.0, 60.0, },
            },
            vertex{
                .position = { 60.0, 60.0, },
                .bulge = -1.0
            },
            vertex{
                .position = { 70.0, 50.0, },
            },
            vertex{
                .position = { 70.0, 20.0 },
                .bulge = 1.0,
            },
            vertex{
                .position = { 60.0, 10.0 },
            },
        },
    });

    return expected_document;
}
