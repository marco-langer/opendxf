// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Marco Langer

#pragma once

namespace odxf {

template <typename... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts>
overload(Ts...) -> overload<Ts...>;

}   // namespace odxf
