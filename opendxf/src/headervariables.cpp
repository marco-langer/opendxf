// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "opendxf/headervariables.hpp"

#include <cmath>
#include <string>
#include <string_view>

namespace {

constexpr std::string_view insUnitsKey{ "$INSUNITS" };
constexpr std::string_view ltScaleKey{ "$LTSCALE" };

}   // namespace

namespace odxf {

std::expected<std::optional<LengthUnit>, HeaderError> insertionUnits(const Header& header)
{
    const auto it{ header.entries.find(insUnitsKey) };
    if (it == header.entries.end()) {
        return std::nullopt;
    }

    const int* value{ std::get_if<int>(&it->second) };
    if (value == nullptr) {
        return std::unexpected(HeaderError::WrongType);
    }

    if (*value < static_cast<int>(LengthUnit::Unitless)
        || *value > static_cast<int>(LengthUnit::USSurveyMile)) {
        return std::unexpected(HeaderError::InvalidValue);
    }

    return static_cast<LengthUnit>(*value);
}

void setInsertionUnits(Header& header, LengthUnit unit)
{
    header.entries.insert_or_assign(std::string{ insUnitsKey }, static_cast<int>(unit));
}

std::expected<std::optional<double>, HeaderError> lineTypeScale(const Header& header)
{
    const auto it{ header.entries.find(ltScaleKey) };
    if (it == header.entries.end()) {
        return std::nullopt;
    }

    const double* value{ std::get_if<double>(&it->second) };
    if (value == nullptr) {
        return std::unexpected(HeaderError::WrongType);
    }

    if (!std::isfinite(*value) || *value <= 0.0) {
        return std::unexpected(HeaderError::InvalidValue);
    }

    return *value;
}

void setLineTypeScale(Header& header, double scale)
{
    header.entries.insert_or_assign(std::string{ ltScaleKey }, scale);
}

}   // namespace odxf
