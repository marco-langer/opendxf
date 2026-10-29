// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/tables.hpp"

namespace odxf {

struct document final
{
    odxf::header header;
    odxf::tables tables;
    odxf::entities entities;
};

}   // namespace odxf
