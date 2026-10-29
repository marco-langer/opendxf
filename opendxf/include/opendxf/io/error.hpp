// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include <optional>
#include <string>

namespace odxf {

enum class error_type
{
    file_open_error,
    invalid_file,
    write_error
};

struct error final
{
    error_type type{ error_type::invalid_file };
    std::optional<int> lineNumber;
    std::string what;
};

}   // namespace odxf
