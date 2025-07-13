#pragma once

#include <optional>
#include <string>

namespace odxf {

struct Error
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
