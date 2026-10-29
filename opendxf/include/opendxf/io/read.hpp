// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/io/error.hpp"

#include <expected>
#include <filesystem>

namespace odxf {

class IReadStream;

std::expected<void, Error> read(const std::filesystem::path& filePath, IReadStream& stream);

}   // namespace odxf
