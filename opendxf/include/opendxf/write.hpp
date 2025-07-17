// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "document.hpp"

#include <filesystem>

namespace odxf {

void writeDxf(const Document& document, const std::filesystem::path& file_path);

}
