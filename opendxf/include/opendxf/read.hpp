#pragma once

#include "error.hpp"

#include <tl/expected.hpp>

#include <filesystem>

namespace odxf {

class IReadStream;

tl::expected<void, Error> read(IReadStream& stream, const std::filesystem::path& filePath);

}   // namespace odxf
