// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include <cstdint>
#include <variant>

namespace odxf {

// The entity takes the property from its layer.
struct ByLayer final
{
    bool operator==(const ByLayer&) const = default;
};

// The entity takes the property from the block reference it's part of.
struct ByBlock final
{
    bool operator==(const ByBlock&) const = default;
};

struct Transparency final
{
    // 0 is fully transparent, 255 opaque.
    std::uint8_t alpha{ 255 };

    bool operator==(const Transparency&) const = default;
};

using EntityTransparency = std::variant<ByLayer, ByBlock, Transparency>;

}   // namespace odxf
