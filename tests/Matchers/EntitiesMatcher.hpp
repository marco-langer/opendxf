#pragma once

#include "opendxf/entities.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::Entities>
AreEntities(const odxf::Entities& expected, double maxError = 1.0e-5);
