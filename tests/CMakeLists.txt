find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
include(GoogleTest)

message(STATUS "Using GTest v.${GTest_VERSION}")

add_executable(opendxf-tests
    Matchers/CoordinateMatcher.cpp
    Matchers/CoordinateMatcher.hpp
    Matchers/DocumentMatcher.cpp
    Matchers/DocumentMatcher.hpp
    Matchers/EntitiesMatcher.cpp
    Matchers/EntitiesMatcher.hpp
    Matchers/HeaderMatcher.cpp
    Matchers/HeaderMatcher.hpp
    Matchers/LayerMatcher.cpp
    Matchers/LayerMatcher.hpp
    Matchers/TablesMatcher.cpp
    Matchers/TablesMatcher.hpp
    read_test.cpp
    TestUtils.cpp
    TestUtils.hpp
    write_test.cpp
)

target_compile_features(opendxf-tests PRIVATE cxx_std_20)

target_compile_definitions(opendxf-tests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

target_link_libraries(opendxf-tests
    PRIVATE
        opendxf
        fmt::fmt
        GTest::gmock_main
)

gtest_discover_tests(opendxf-tests)
