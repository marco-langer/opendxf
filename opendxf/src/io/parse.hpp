// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include "standard_arithmetic.hpp"

#include <charconv>
#include <optional>
#include <string_view>

namespace odxf {

// Parses a group code or a numeric value. Leading spaces are skipped because DXF writers
// right-justify group codes and integers, anything after the number is an error.
template <standard_arithmetic T>
std::optional<T> parse_as(std::string_view value)
{
    const char* first{ value.data() };
    const char* last{ value.data() + value.size() };
    while (first != last && *first == ' ') {
        ++first;
    }

    T result;
    const auto [ptr, error_code]{ std::from_chars(first, last, result) };

    return error_code == std::errc() && ptr == last ? result : std::optional<T>{};
}

}   // namespace odxf
