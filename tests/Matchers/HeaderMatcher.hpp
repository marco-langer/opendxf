#pragma once

#include "opendxf/header.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::Header> IsHeader(const odxf::Header& expected);
