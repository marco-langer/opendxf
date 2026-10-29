// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "opendxf/header_variables.hpp"

#include <cmath>
#include <string>
#include <string_view>

namespace {

constexpr std::string_view insUnitsKey{ "$INSUNITS" };
constexpr std::string_view ltScaleKey{ "$LTSCALE" };

}   // namespace

namespace odxf {

std::expected<std::optional<length_unit>, header_error> insertionUnits(const odxf::header& header)
{
    const auto it{ header.entries.find(insUnitsKey) };
    if (it == header.entries.end()) {
        return std::nullopt;
    }

    const int* value{ std::get_if<int>(&it->second) };
    if (value == nullptr) {
        return std::unexpected(header_error::wrong_type);
    }

    if (*value < static_cast<int>(length_unit::unitless)
        || *value > static_cast<int>(length_unit::us_survey_mile)) {
        return std::unexpected(header_error::invalid_value);
    }

    return static_cast<length_unit>(*value);
}

void setInsertionUnits(odxf::header& header, length_unit unit)
{
    header.entries.insert_or_assign(std::string{ insUnitsKey }, static_cast<int>(unit));
}

std::expected<std::optional<double>, header_error> lineTypeScale(const odxf::header& header)
{
    const auto it{ header.entries.find(ltScaleKey) };
    if (it == header.entries.end()) {
        return std::nullopt;
    }

    const double* value{ std::get_if<double>(&it->second) };
    if (value == nullptr) {
        return std::unexpected(header_error::wrong_type);
    }

    if (!std::isfinite(*value) || *value <= 0.0) {
        return std::unexpected(header_error::invalid_value);
    }

    return *value;
}

void setLineTypeScale(odxf::header& header, double scale)
{
    header.entries.insert_or_assign(std::string{ ltScaleKey }, scale);
}

}   // namespace odxf
