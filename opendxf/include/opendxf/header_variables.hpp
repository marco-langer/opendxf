// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include "opendxf/header.hpp"

#include <expected>
#include <optional>

namespace odxf {

// errors for header variables that are present but malformed;
// an absent optional variable is reported as std::nullopt instead
enum class header_error
{
    wrong_type,
    invalid_value
};

// values as stored in $INSUNITS
enum class length_unit
{
    unitless = 0,
    inches = 1,
    feet = 2,
    miles = 3,
    millimeters = 4,
    centimeters = 5,
    meters = 6,
    kilometers = 7,
    microinches = 8,
    mils = 9,
    yards = 10,
    angstroms = 11,
    nanometers = 12,
    microns = 13,
    decimeters = 14,
    decameters = 15,
    hectometers = 16,
    gigameters = 17,
    astronomical_units = 18,
    light_years = 19,
    parsecs = 20,
    us_survey_feet = 21,
    us_survey_inch = 22,
    us_survey_yard = 23,
    us_survey_mile = 24
};

std::expected<std::optional<length_unit>, header_error> insertionUnits(const odxf::header& header);

void setInsertionUnits(odxf::header& header, length_unit unit);

// $LTSCALE, the scale of all line type patterns. A scale that isn't positive is an invalid_value.
std::expected<std::optional<double>, header_error> lineTypeScale(const odxf::header& header);

void setLineTypeScale(odxf::header& header, double scale);

}   // namespace odxf
