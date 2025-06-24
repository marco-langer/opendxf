#include "DocumentMatcher.hpp"

#include "HeaderMatcher.hpp"

testing::Matcher<odxf::Document> IsDocument(const odxf::Document& expected)
{
    return testing::AllOf(
        testing::Field("header", &odxf::Document::header, IsHeader(expected.header)));
}
