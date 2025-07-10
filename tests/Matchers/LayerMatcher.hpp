#pragma once

#include "opendxf/layer.hpp"

#include <gmock/gmock.h>

testing::Matcher<std::vector<odxf::Layer>> AreLayers(const std::vector<odxf::Layer>& expected);
