// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/write.hpp"

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
            overload{ [&stream](int element) {
                         stream << "\n70\n";
                         stream << element;
                     },
                      [&stream](const std::string& element) {
                          stream << "\n1\n";
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
    stream << "\n0\nLTYPE";
    stream << "\n2\n" << lineType.name;
    stream << "\n70\n" << lineType.flags;
    stream << "\n3\n" << lineType.displayName;
    stream << "\n72\n65";
    stream << "\n73\n0";
    stream << "\n40\n0";
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

void writeLayer(std::ofstream& stream, const odxf::Layer& layer, const std::string& lineTypeName)
{
    stream << "\n0\nLAYER";
    stream << "\n2\n" << layer.name;
    stream << "\n70\n" << layer.flags;
    stream << "\n62\n" << layer.color;
    stream << "\n6\n" << lineTypeName;
}

void writeLayers(
    std::ofstream& stream,
    const std::vector<odxf::Layer>& layers,
    const std::vector<odxf::LineType>& lineTypes)
{
    stream << "\n0\nTABLE";
    stream << "\n2\nLAYER";
    stream << "\n70\n" << layers.size();
    for (const odxf::Layer& layer : layers) {
        writeLayer(stream, layer, lineTypes.at(layer.lineType).name);
    }
    stream << "\n0\nENDTAB";
}

void writeTables(std::ofstream& stream, const odxf::Tables& tables)
{
    stream << "\n0\nSECTION";
    stream << "\n2\nTABLES";
    writeLineTypes(stream, tables.lineTypes);
    writeLayers(stream, tables.layers, tables.lineTypes);
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
    stream << "\n62\n" << point.color;
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
    stream << "\n62\n" << ray.color;
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
    stream << "\n62\n" << line.color;
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
    stream << "\n62\n" << circle.color;
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
    stream << "\n62\n" << arc.color;
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
    stream << "\n62\n" << ellipse.color;
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
    stream << "\n62\n" << lwPolyline.color;
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
