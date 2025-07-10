#include "HeaderMatcher.hpp"

testing::Matcher<odxf::Header> IsHeader(const odxf::Header& expected)
{
    return testing::Field("entries", &odxf::Header::entries, expected.entries);
}
