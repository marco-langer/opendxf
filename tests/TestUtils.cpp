// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "TestUtils.hpp"

#include "opendxf/coordinate.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/layer.hpp"

odxf::Document createExampleDocument()
{
    using namespace odxf;

    Document expectedDocument;
    auto& headerEntries{ expectedDocument.header.entries };
    headerEntries.try_emplace("$LIMMAX", odxf::Coordinate2d{ 420, 297 });
    headerEntries.try_emplace("$LIMMIN", odxf::Coordinate2d{});
    headerEntries.try_emplace(
        "$EXTMIN", odxf::Coordinate3d{ 77.48842691804612, 27.48842691804612, 0.0 });
    headerEntries.try_emplace("$INSBASE", odxf::Coordinate3d{});
    headerEntries.try_emplace("$ACADVER", "AC1032");

    const std::string layerNameTest{ "Test Layer" };
    const std::string layerNameLines{ "Lines" };
    const std::string layerNameLWPolylines{ "LW Polylines" };

    LineTypes& lineTypes{ expectedDocument.tables.lineTypes };
    lineTypes.push_back(LineType{
        .name = "CONTINUOUS",
        .displayName = "Solid Line",
    });

    Layers& layers{ expectedDocument.tables.layers };
    layers.emplace_back(Layer{ .name = layerNameTest }).flags = Layer::Flags::Frozen;
    layers.push_back(Layer{ .name = layerNameLines });
    layers.push_back(Layer{ .name = layerNameLWPolylines });

    Entities& entities{ expectedDocument.entities };

    Lines& lines{ entities.lines };
    lines
        .emplace_back(Line{
            .start = Coordinate3d{ 0.0, 0.5, 0.0 },
            .end = Coordinate3d{ 1.0, 1.5, 0.0 },
        })
        .layer = layerNameLines;

    lines
        .emplace_back(Line{
            .start = Coordinate3d{ 0.0, 0.0, 0.0 },
            .end = Coordinate3d{ 1.0, 1.0, 0.0 },
        })
        .layer = layerNameLines;

    Circles& circles{ entities.circles };
    circles
        .emplace_back(Circle{
            .center = Coordinate3d{ 0.0, 0.0, 0.0 },
            .radius = 1.0,
        })
        .layer = layerNameTest;

    Arcs& arcs{ entities.arcs };
    arcs.emplace_back(Arc{
                          .center = Coordinate3d{ 0.0, 2.0, 0.0 },
                          .radius = 0.5,
                          .startAngle = 0.0,
                          .endAngle = 180.0,
                      })
        .layer = layerNameTest;

    LWPolylines& lwPolylines{ entities.lwPolylines };
    lwPolylines.emplace_back(LWPolyline{}).layer = layerNameLWPolylines;
    lwPolylines
        .emplace_back(LWPolyline{
            .vertices = { Vertex{ .position = { 200.0, 200.0 } } },
        })
        .layer = layerNameLWPolylines;
    lwPolylines.emplace_back(LWPolyline{
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
    }).layer = layerNameLWPolylines;

    return expectedDocument;
}
