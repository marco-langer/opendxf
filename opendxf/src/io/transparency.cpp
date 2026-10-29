// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "transparency.hpp"

namespace odxf {

std::optional<transparency> decodeLayerTransparency(int value)
{
    if ((value & ~0xFF) != byValueTransparencyFlag) {
        // TODO: warn about the ignored value once opendxf can report warnings.
        return std::nullopt;
    }

    return transparency{ .alpha = static_cast<std::uint8_t>(value & 0xFF) };
}

entity_transparency decodeEntityTransparency(int value)
{
    if (value == byBlockTransparencyValue) {
        return by_block{};
    }

    if ((value & ~0xFF) != byValueTransparencyFlag) {
        // TODO: warn about the ignored value once opendxf can report warnings.
        return by_layer{};
    }

    return transparency{ .alpha = static_cast<std::uint8_t>(value & 0xFF) };
}

int encodeTransparency(const odxf::transparency& transparency)
{
    return byValueTransparencyFlag | transparency.alpha;
}

}   // namespace odxf
