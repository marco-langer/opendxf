// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "opendxf/header_variables.hpp"

#include <cmath>
#include <string>
#include <string_view>

namespace {

constexpr std::string_view ins_units_key{ "$INSUNITS" };
constexpr std::string_view lt_scale_key{ "$LTSCALE" };

}   // namespace

namespace odxf {

std::expected<std::optional<length_unit>, header_error> insertion_units(const odxf::header& header)
{
    const auto it{ header.entries.find(ins_units_key) };
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

void set_insertion_units(odxf::header& header, length_unit unit)
{
    header.entries.insert_or_assign(std::string{ ins_units_key }, static_cast<int>(unit));
}

std::expected<std::optional<double>, header_error> line_type_scale(const odxf::header& header)
{
    const auto it{ header.entries.find(lt_scale_key) };
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

void set_line_type_scale(odxf::header& header, double scale)
{
    header.entries.insert_or_assign(std::string{ lt_scale_key }, scale);
}

}   // namespace odxf
