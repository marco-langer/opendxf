// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/document.hpp"
#include "opendxf/io/error.hpp"

#include <expected>
#include <filesystem>

namespace odxf {

class read_stream_base;

std::expected<document, error> read(const std::filesystem::path& file_path);

std::expected<void, error> read(const std::filesystem::path& file_path, read_stream_base& stream);

}   // namespace odxf
