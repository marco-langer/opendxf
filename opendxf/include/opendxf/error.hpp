#pragma once

#include <string>

namespace odxf {

struct Error
{
    enum class Type
    {
        FileOpenError,
        InvalidFile
    };

    Type type;
    std::string what;
};

}   // namespace odxf
