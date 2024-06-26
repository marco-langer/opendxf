#pragma once

#include "entities.hpp"
#include "header.hpp"
#include "tables.hpp"

namespace odxf {

struct Document
{
    Header header;
    Tables tables;
    Entities entities;
};

}   // namespace odxf
