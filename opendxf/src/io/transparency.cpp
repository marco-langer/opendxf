// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#include "transparency.hpp"

namespace odxf {

std::optional<Transparency> decodeLayerTransparency(int value)
{
    if ((value & ~0xFF) != byValueTransparencyFlag) {
        // TODO: warn about the ignored value once opendxf can report warnings.
        return std::nullopt;
    }

    return Transparency{ .alpha = static_cast<std::uint8_t>(value & 0xFF) };
}

EntityTransparency decodeEntityTransparency(int value)
{
    if (value == byBlockTransparencyValue) {
        return ByBlock{};
    }

    if ((value & ~0xFF) != byValueTransparencyFlag) {
        // TODO: warn about the ignored value once opendxf can report warnings.
        return ByLayer{};
    }

    return Transparency{ .alpha = static_cast<std::uint8_t>(value & 0xFF) };
}

int encodeTransparency(const Transparency& transparency)
{
    return byValueTransparencyFlag | transparency.alpha;
}

}   // namespace odxf
