// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

#include <concepts>

namespace odxf {

// the arithmetic types std::to_chars and std::from_chars accept; neither takes bool
template <typename T>
concept standard_arithmetic =
    (std::integral<T> || std::floating_point<T>) && !std::same_as<T, bool>;

}   // namespace odxf
