// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include "opendxf/properties.hpp"

#include <optional>
#include <string_view>

namespace odxf {

// The registered application under which DXF files store a layer's transparency in its xdata.
inline constexpr std::string_view layer_transparency_app{ "AcCmTransparency" };

// Group code 440 and the layer xdata store a transparency as 0x02000000 | alpha, entities can also
// be 0x01000000 (BYBLOCK).
inline constexpr int by_block_transparency_value{ 0x01000000 };
inline constexpr int by_value_transparency_flag{ 0x02000000 };

// For a layer: empty, i.e. opaque, for anything but 0x02000000 | alpha.
std::optional<transparency> decode_layer_transparency(int value);

// BYLAYER for anything but 0x01000000 or 0x02000000 | alpha.
entity_transparency decode_entity_transparency(int value);

int encode_transparency(const odxf::transparency& transparency);

}   // namespace odxf
