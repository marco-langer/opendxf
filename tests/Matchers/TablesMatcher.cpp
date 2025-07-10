#include "TablesMatcher.hpp"

#include "LayerMatcher.hpp"

testing::Matcher<odxf::Tables> AreTables(const odxf::Tables& expected)
{
    return testing::AllOf(
        testing::Field("layers", &odxf::Tables::layers, AreLayers(expected.layers)));
}
