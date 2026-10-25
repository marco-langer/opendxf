// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/tables.hpp"

namespace odxf {

struct Document final
{
    Header header;
    Tables tables;
    Entities entities;
};

}   // namespace odxf
