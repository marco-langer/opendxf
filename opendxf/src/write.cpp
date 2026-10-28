// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/write.hpp"

#include "transparency.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>

namespace {

template <typename... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts>
overload(Ts...) -> overload<Ts...>;

void writeHeader(std::ofstream& stream, const odxf::Header& header)
{
    stream << "999\nopendxf";
    stream << "\n0\nSECTION";
    stream << "\n2\nHEADER";

    for (const auto& [key, value] : header.entries) {
        stream << "\n9\n" << key;
        std::visit(
            overload{ [&stream, &key](bool element) {
                         stream << "\n290\n";
                         stream << element;
                     },
                      [&stream, &key](int element) {
                          if (key == "$CECOLOR" || key == "$INTERFERECOLOR") {
                              stream << "\n62\n";
                          } else if (
                              key == "$ENDCAPS" || key == "$JOINSTYLE" || key == "$SORTENTS"
                              || key == "$INDEXCTL" || key == "$HIDETEXT" || key == "$HALOGAP"
                              || key == "$OBSLTYPE" || key == "$INTERSECTIONDISPLAY"
                              || key == "$DIMASSOC" || key == "$LOFTNORMALS"
                              || key == "$LIGHTGLYPHDISPLAY" || key == "$TILEMODELIGHTSYNCH"
                              || key == "$SOLIDHIST" || key == "$SHOWHIST" || key == "$DWFFRAME"
                              || key == "$DGNFRAME" || key == "$CSHADOW") {
                              stream << "\n280\n";
                          } else if (key == "$CELWEIGHT") {
                              stream << "\n370\n";
                          } else if (key == "$CEPSNTYPE") {
                              stream << "\n380\n";
                          } else {
                              stream << "\n70\n";
                          }
                          stream << element;
                      },
                      [&stream, &key](double element) {
                          if (key == "$ANGBASE") {
                              stream << "\n50\n";
                          } else {
                              stream << "\n40\n";
                          }
                          stream << element;
                      },
                      [&stream, &key](const std::string& element) {
                          if (key == "$DIMSTYLE") {
                              stream << "\n2\n";
                          } else if (key == "$HANDSEED") {
                              stream << "\n5\n";
                          } else if (
                              key == "$CELTYPE" || key == "$DIMLTYPE" || key == "$DIMLTEX1"
                              || key == "$DIMLTEX2") {
                              stream << "\n6\n";
                          } else if (key == "$TEXTSTYLE" || key == "$DIMTXSTY") {
                              stream << "\n7\n";
                          } else if (key == "$CLAYER") {
                              stream << "\n8\n";
                          } else {
                              stream << "\n1\n";
                          }
                          stream << element;
                      },
                      [&stream](const odxf::Coordinate2d& coord) {
                          stream << "\n10\n";
                          stream << coord.x;
                          stream << "\n20\n";
                          stream << coord.y;
                      },
                      [&stream](const odxf::Coordinate3d& coord) {
                          stream << "\n10\n";
                          stream << coord.x;
                          stream << "\n20\n";
                          stream << coord.y;
                          stream << "\n30\n";
                          stream << coord.z;
                      } },
            value);
    }

    stream << "\n0\nENDSEC";
}

void writeLineType(std::ofstream& stream, const odxf::LineType& lineType)
{
    double patternLength{ 0.0 };
    for (const odxf::LineTypeElement& element : lineType.elements) {
        patternLength += std::abs(element.length);
    }

    stream << "\n0\nLTYPE";
    stream << "\n2\n" << lineType.name;
    stream << "\n70\n" << lineType.flags;
    stream << "\n3\n" << lineType.displayName;
    stream << "\n72\n65";
    stream << "\n73\n" << lineType.elements.size();
    stream << "\n40\n" << patternLength;
    for (const odxf::LineTypeElement& element : lineType.elements) {
        stream << "\n49\n" << element.length;
        stream << "\n74\n" << element.complexFlags;
    }
}

void writeLineTypes(std::ofstream& stream, const std::vector<odxf::LineType>& lineTypes)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLTYPE";
    stream << "\n70\n" << lineTypes.size();
    for (const odxf::LineType& lineType : lineTypes) {
        writeLineType(stream, lineType);
    }
    stream << "\n0\nENDTAB";
}

void writeTrueColor(std::ofstream& stream, const std::optional<int>& maybeTrueColor)
{
    if (maybeTrueColor) {
        stream << "\n420\n" << *maybeTrueColor;
    }
}

// BYLAYER is written as no group code 440.
void writeTransparency(std::ofstream& stream, const odxf::EntityTransparency& transparency)
{
    if (std::holds_alternative<odxf::ByBlock>(transparency)) {
        stream << "\n440\n" << odxf::byBlockTransparencyValue;
    } else if (const auto* byValue{ std::get_if<odxf::Transparency>(&transparency) }) {
        stream << "\n440\n" << odxf::encodeTransparency(*byValue);
    }
}

void writeLayer(std::ofstream& stream, const odxf::Layer& layer)
{
    stream << "\n0\nLAYER";
    stream << "\n2\n" << layer.name;
    stream << "\n70\n" << layer.flags;
    stream << "\n62\n" << layer.color;
    writeTrueColor(stream, layer.trueColor);
    stream << "\n6\n" << layer.lineType;
    stream << "\n370\n" << layer.lineweight;
    // xdata comes last in a record
    if (layer.transparency) {
        stream << "\n1001\n" << odxf::layerTransparencyApp;
        stream << "\n1071\n" << odxf::encodeTransparency(*layer.transparency);
    }
}

void writeLayers(std::ofstream& stream, const std::vector<odxf::Layer>& layers)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLAYER";
    stream << "\n70\n" << layers.size();
    for (const odxf::Layer& layer : layers) {
        writeLayer(stream, layer);
    }
    stream << "\n0\nENDTAB";
}

// Every application that owns xdata in the file needs an entry in the APPID table.
void writeAppIds(std::ofstream& stream, const std::vector<odxf::Layer>& layers)
{
    const bool hasLayerTransparency{ std::ranges::any_of(
        layers, [](const odxf::Layer& layer) { return layer.transparency.has_value(); }) };
    if (!hasLayerTransparency) {
        return;
    }

    stream << "\n0\nTABLE";
    stream << "\n2\nAPPID";
    stream << "\n70\n1";
    stream << "\n0\nAPPID";
    stream << "\n2\n" << odxf::layerTransparencyApp;
    stream << "\n70\n0";
    stream << "\n0\nENDTAB";
}

void writeTables(std::ofstream& stream, const odxf::Tables& tables)
{
    stream << "\n0\nSECTION";
    stream << "\n2\nTABLES";
    writeLineTypes(stream, tables.lineTypes);
    writeLayers(stream, tables.layers);
    writeAppIds(stream, tables.layers);
    stream << "\n0\nENDSEC";
}

void writeBlocks(std::ofstream& stream)
{
    stream << "\n0\nSECTION";
    stream << "\n2\nBLOCKS";
    stream << "\n0\nENDSEC";
}

void writeCoordinate(std::ofstream& stream, const odxf::Coordinate3d& coordinate)
{
    stream << "\n10\n" << std::to_string(coordinate.x);
    stream << "\n20\n" << std::to_string(coordinate.y);
    stream << "\n30\n" << std::to_string(coordinate.z);
}

void writeThickness(std::ofstream& stream, const std::optional<double>& maybeThickness)
{
    if (maybeThickness) {
        stream << "\n39\n" << *maybeThickness;
    }
}

void writeExtrusion(std::ofstream& stream, const std::optional<odxf::Vector3d>& maybeExtrusion)
{
    if (maybeExtrusion) {
        const odxf::Vector3d& extrusion{ *maybeExtrusion };
        stream << "\n210\n" << std::to_string(extrusion.x);
        stream << "\n220\n" << std::to_string(extrusion.y);
        stream << "\n230\n" << std::to_string(extrusion.z);
    }
}

void writePoint(std::ofstream& stream, const odxf::Point& point)
{
    stream << "\n0\nLINE\n100\nAcDbPoint";
    stream << "\n8\n" << point.layer;
    stream << "\n6\n" << point.lineType;
    stream << "\n62\n" << point.color;
    stream << "\n370\n" << point.lineweight;
    stream << "\n48\n" << point.lineTypeScale;
    writeTrueColor(stream, point.trueColor);
    writeTransparency(stream, point.transparency);
    writeThickness(stream, point.thickness);
    stream << "\n10\n" << std::to_string(point.coordinate.x);
    stream << "\n20\n" << std::to_string(point.coordinate.y);
    stream << "\n30\n" << std::to_string(point.coordinate.z);
    writeExtrusion(stream, point.extrusion);
}


void writeRay(std::ofstream& stream, const odxf::Ray& ray)
{
    stream << "\n0\nLINE\n100\nAcDbRay";
    stream << "\n8\n" << ray.layer;
    stream << "\n6\n" << ray.lineType;
    stream << "\n62\n" << ray.color;
    stream << "\n370\n" << ray.lineweight;
    stream << "\n48\n" << ray.lineTypeScale;
    writeTrueColor(stream, ray.trueColor);
    writeTransparency(stream, ray.transparency);
    stream << "\n10\n" << std::to_string(ray.startPoint.x);
    stream << "\n20\n" << std::to_string(ray.startPoint.y);
    stream << "\n30\n" << std::to_string(ray.startPoint.z);
    stream << "\n11\n" << std::to_string(ray.direction.x);
    stream << "\n21\n" << std::to_string(ray.direction.y);
    stream << "\n31\n" << std::to_string(ray.direction.z);
}

void writeLine(std::ofstream& stream, const odxf::Line& line)
{
    stream << "\n0\nLINE\n100\nAcDbLine";
    stream << "\n8\n" << line.layer;
    stream << "\n6\n" << line.lineType;
    stream << "\n62\n" << line.color;
    stream << "\n370\n" << line.lineweight;
    stream << "\n48\n" << line.lineTypeScale;
    writeTrueColor(stream, line.trueColor);
    writeTransparency(stream, line.transparency);
    writeThickness(stream, line.thickness);
    stream << "\n10\n" << std::to_string(line.start.x);
    stream << "\n20\n" << std::to_string(line.start.y);
    stream << "\n30\n" << std::to_string(line.start.z);
    stream << "\n11\n" << std::to_string(line.end.x);
    stream << "\n21\n" << std::to_string(line.end.y);
    stream << "\n31\n" << std::to_string(line.end.z);
    writeExtrusion(stream, line.extrusion);
}

void writeCircle(std::ofstream& stream, const odxf::Circle& circle)
{
    stream << "\n0\nCIRCLE\n100\nAcDbCircle";
    stream << "\n8\n" << circle.layer;
    stream << "\n6\n" << circle.lineType;
    stream << "\n62\n" << circle.color;
    stream << "\n370\n" << circle.lineweight;
    stream << "\n48\n" << circle.lineTypeScale;
    writeTrueColor(stream, circle.trueColor);
    writeTransparency(stream, circle.transparency);
    writeThickness(stream, circle.thickness);
    stream << "\n10\n" << std::to_string(circle.center.x);
    stream << "\n20\n" << std::to_string(circle.center.y);
    stream << "\n30\n" << std::to_string(circle.center.z);
    stream << "\n40\n" << std::to_string(circle.radius);
    writeExtrusion(stream, circle.extrusion);
}

void writeArc(std::ofstream& stream, const odxf::Arc& arc)
{
    stream << "\n0\nARC\n100\nAcDbCircle";
    stream << "\n8\n" << arc.layer;
    stream << "\n6\n" << arc.lineType;
    stream << "\n62\n" << arc.color;
    stream << "\n370\n" << arc.lineweight;
    stream << "\n48\n" << arc.lineTypeScale;
    writeTrueColor(stream, arc.trueColor);
    writeTransparency(stream, arc.transparency);
    writeThickness(stream, arc.thickness);
    stream << "\n10\n" << std::to_string(arc.center.x);
    stream << "\n20\n" << std::to_string(arc.center.y);
    stream << "\n30\n" << std::to_string(arc.center.z);
    stream << "\n40\n" << std::to_string(arc.radius);
    stream << "\n100\nAcDbArc";
    stream << "\n50\n" << std::to_string(arc.startAngle);
    stream << "\n51\n" << std::to_string(arc.endAngle);
    writeExtrusion(stream, arc.extrusion);
}

void writeEllipse(std::ofstream& stream, const odxf::Ellipse& ellipse)
{
    stream << "\n0\nARC\n100\nAcDbEllipse";
    stream << "\n8\n" << ellipse.layer;
    stream << "\n6\n" << ellipse.lineType;
    stream << "\n62\n" << ellipse.color;
    stream << "\n370\n" << ellipse.lineweight;
    stream << "\n48\n" << ellipse.lineTypeScale;
    writeTrueColor(stream, ellipse.trueColor);
    writeTransparency(stream, ellipse.transparency);
    stream << "\n10\n" << std::to_string(ellipse.center.x);
    stream << "\n20\n" << std::to_string(ellipse.center.y);
    stream << "\n30\n" << std::to_string(ellipse.center.z);
    stream << "\n11\n" << std::to_string(ellipse.endPointMajor.x);
    stream << "\n21\n" << std::to_string(ellipse.endPointMajor.y);
    stream << "\n31\n" << std::to_string(ellipse.endPointMajor.z);
    stream << "\n40\n" << std::to_string(ellipse.axisRatio);
    stream << "\n41\n" << std::to_string(ellipse.startParameter);
    stream << "\n42\n" << std::to_string(ellipse.endParameter);
    writeExtrusion(stream, ellipse.extrusion);
}

void writeLWPolyline(std::ofstream& stream, const odxf::LWPolyline& lwPolyline)
{
    stream << "\n0\nLWPOLYLINE\n100\nAcDbPolyline";
    stream << "\n8\n" << lwPolyline.layer;
    stream << "\n6\n" << lwPolyline.lineType;
    stream << "\n62\n" << lwPolyline.color;
    stream << "\n370\n" << lwPolyline.lineweight;
    stream << "\n48\n" << lwPolyline.lineTypeScale;
    writeTrueColor(stream, lwPolyline.trueColor);
    writeTransparency(stream, lwPolyline.transparency);
    stream << "\n90\n" << lwPolyline.vertices.size();
    stream << "\n70\n" << lwPolyline.isClosed ? 1 : 0;

    for (const odxf::Vertex& vertex : lwPolyline.vertices) {
        stream << "\n10\n" << std::to_string(vertex.position.x);
        stream << "\n20\n" << std::to_string(vertex.position.y);
        if (vertex.bulge.has_value()) {
            stream << "\n42\n" << std::to_string(*vertex.bulge);
        }
    }
}

void writeEntities(
    std::ofstream& stream, const odxf::Entities& entities, const odxf::Tables& tables)
{
    stream << "\n0\nSECTION\n2\nENTITIES";

    for (const odxf::Point& point : entities.points) {
        writePoint(stream, point);
    }

    for (const odxf::Ray& ray : entities.rays) {
        writeRay(stream, ray);
    }

    for (const odxf::Line& line : entities.lines) {
        writeLine(stream, line);
    }

    for (const odxf::Circle& circle : entities.circles) {
        writeCircle(stream, circle);
    }

    for (const odxf::Arc& arc : entities.arcs) {
        writeArc(stream, arc);
    }

    for (const odxf::Ellipse& ellipse : entities.ellipses) {
        writeEllipse(stream, ellipse);
    }

    for (const odxf::LWPolyline& lwPolyline : entities.lwPolylines) {
        writeLWPolyline(stream, lwPolyline);
    }

    stream << "\n0\nENDSEC";
}

void writeEof(std::ofstream& stream) { stream << "\n0\nEOF"; }

}   // namespace

namespace odxf {

void writeDxf(const Document& document, const std::filesystem::path& file_path)
{
    std::ofstream stream{ file_path };
    if (!stream.is_open()) {
        return;
    }

    stream << std::setprecision(16);

    writeHeader(stream, document.header);
    writeTables(stream, document.tables);
    writeBlocks(stream);
    writeEntities(stream, document.entities, document.tables);
    writeEof(stream);
}

}   // namespace odxf
