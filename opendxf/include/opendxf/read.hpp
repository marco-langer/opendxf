// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "error.hpp"

#include <expected>
#include <filesystem>

namespace odxf {

class IReadStream;

std::expected<void, Error> read(IReadStream& stream, const std::filesystem::path& filePath);

}   // namespace odxf
