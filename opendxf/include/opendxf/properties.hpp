// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include <cstdint>
#include <variant>

namespace odxf {

// The entity takes the property from its layer.
struct by_layer final
{
    bool operator==(const by_layer&) const = default;
};

// The entity takes the property from the block reference it's part of.
struct by_block final
{
    bool operator==(const by_block&) const = default;
};

struct transparency final
{
    // 0 is fully transparent, 255 opaque.
    std::uint8_t alpha{ 255 };

    bool operator==(const transparency&) const = default;
};

using entity_transparency = std::variant<by_layer, by_block, transparency>;

}   // namespace odxf
