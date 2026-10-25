// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "opendxf/headervariables.hpp"

#include <string>
#include <string_view>

namespace {

constexpr std::string_view insUnitsKey{ "$INSUNITS" };

}   // namespace

namespace odxf {

std::expected<LengthUnit, HeaderError> insertionUnits(const Header& header)
{
    const auto it{ header.entries.find(insUnitsKey) };
    if (it == header.entries.end()) {
        return std::unexpected(HeaderError::MissingVariable);
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

}   // namespace odxf
