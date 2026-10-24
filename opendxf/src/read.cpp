// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#include "opendxf/read.hpp"

#include "opendxf/ireadstream.hpp"
#include "reader.hpp"

namespace odxf {

std::expected<void, Error> read(IReadStream& stream, const std::filesystem::path& filePath)
{
    Reader reader{ stream };

    return reader.readAll(filePath);
}

}   // namespace odxf
