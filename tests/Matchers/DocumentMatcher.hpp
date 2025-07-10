#pragma once

#include "opendxf/document.hpp"

#include <gmock/gmock.h>

testing::Matcher<odxf::Document> IsDocument(const odxf::Document& expected);
