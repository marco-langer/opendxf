// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "transparency.hpp"

namespace odxf {

std::optional<transparency> decode_layer_transparency(int value)
{
    if ((value & ~0xFF) != by_value_transparency_flag) {
        // TODO: warn about the ignored value once opendxf can report warnings.
        return std::nullopt;
    }

    return transparency{ .alpha = static_cast<std::uint8_t>(value & 0xFF) };
}

entity_transparency decode_entity_transparency(int value)
{
    if (value == by_block_transparency_value) {
        return by_block{};
    }

    if ((value & ~0xFF) != by_value_transparency_flag) {
        // TODO: warn about the ignored value once opendxf can report warnings.
        return by_layer{};
    }

    return transparency{ .alpha = static_cast<std::uint8_t>(value & 0xFF) };
}

int encode_transparency(const odxf::transparency& transparency)
{
    return by_value_transparency_flag | transparency.alpha;
}

}   // namespace odxf
