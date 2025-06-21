find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
include(GoogleTest)

add_executable(opendxf-tests
    write_test.cpp
)

target_compile_features(opendxf-tests PRIVATE cxx_std_20)

target_compile_definitions(opendxf-tests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

target_link_libraries(opendxf-tests
    PRIVATE
        opendxf
        fmt::fmt-header-only
        GTest::gmock_main
)

gtest_discover_tests(opendxf-tests)
