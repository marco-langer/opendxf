// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include "opendxf/properties.hpp"

#include <optional>
#include <string_view>

namespace odxf {

// The registered application under which DXF files store a layer's transparency in its xdata.
inline constexpr std::string_view layerTransparencyApp{ "AcCmTransparency" };

// Group code 440 and the layer xdata store a transparency as 0x02000000 | alpha, entities can also
// be 0x01000000 (BYBLOCK).
inline constexpr int byBlockTransparencyValue{ 0x01000000 };
inline constexpr int byValueTransparencyFlag{ 0x02000000 };

// For a layer: empty, i.e. opaque, for anything but 0x02000000 | alpha.
std::optional<Transparency> decodeLayerTransparency(int value);

// BYLAYER for anything but 0x01000000 or 0x02000000 | alpha.
EntityTransparency decodeEntityTransparency(int value);

int encodeTransparency(const Transparency& transparency);

}   // namespace odxf
