#pragma once

#include "opendxf/tables.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::Tables> AreTables(const odxf::Tables& expected);
