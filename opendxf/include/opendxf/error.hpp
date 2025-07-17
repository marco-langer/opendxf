// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include <optional>
#include <string>

namespace odxf {

struct Error final
{
    enum class Type
    {
        FileOpenError,
        InvalidFile
    };

    Type type{ Type::InvalidFile };
    std::optional<int> lineNumber;
    std::string what;
};

}   // namespace odxf
