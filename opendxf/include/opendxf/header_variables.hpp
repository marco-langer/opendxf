// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include "opendxf/header.hpp"

#include <expected>
#include <optional>

namespace odxf {

// errors for header variables that are present but malformed;
// an absent optional variable is reported as std::nullopt instead
enum class HeaderError
{
    WrongType,
    InvalidValue
};

// values as stored in $INSUNITS
enum class LengthUnit
{
    Unitless = 0,
    Inches = 1,
    Feet = 2,
    Miles = 3,
    Millimeters = 4,
    Centimeters = 5,
    Meters = 6,
    Kilometers = 7,
    Microinches = 8,
    Mils = 9,
    Yards = 10,
    Angstroms = 11,
    Nanometers = 12,
    Microns = 13,
    Decimeters = 14,
    Decameters = 15,
    Hectometers = 16,
    Gigameters = 17,
    AstronomicalUnits = 18,
    LightYears = 19,
    Parsecs = 20,
    USSurveyFeet = 21,
    USSurveyInch = 22,
    USSurveyYard = 23,
    USSurveyMile = 24
};

std::expected<std::optional<LengthUnit>, HeaderError> insertionUnits(const Header& header);

void setInsertionUnits(Header& header, LengthUnit unit);

// $LTSCALE, the scale of all line type patterns. A scale that isn't positive is an InvalidValue.
std::expected<std::optional<double>, HeaderError> lineTypeScale(const Header& header);

void setLineTypeScale(Header& header, double scale);

}   // namespace odxf
