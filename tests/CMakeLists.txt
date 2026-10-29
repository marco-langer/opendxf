include(FetchContent)
include(GoogleTest)

# uses an installed GTest if available, otherwise downloads and builds it
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.tar.gz
    URL_HASH SHA256=6e3191c1455468b3fc35a417fb565c1c5071aee1b7e7f85e30cf48a98d37d8b5
    FIND_PACKAGE_ARGS NAMES GTest CONFIG
)
set(INSTALL_GTEST OFF)
set(gtest_force_shared_crt ON)
FetchContent_MakeAvailable(googletest)

if(googletest_SOURCE_DIR)
    message(STATUS "Using downloaded GTest from ${googletest_SOURCE_DIR}")
else()
    message(STATUS "Using installed GTest")
endif()

add_executable(opendxf-tests
    matchers/coordinate_matcher.cpp
    matchers/coordinate_matcher.hpp
    matchers/document_matcher.cpp
    matchers/document_matcher.hpp
    matchers/entities_matcher.cpp
    matchers/entities_matcher.hpp
    matchers/header_matcher.cpp
    matchers/header_matcher.hpp
    matchers/layer_matcher.cpp
    matchers/layer_matcher.hpp
    matchers/tables_matcher.cpp
    matchers/tables_matcher.hpp
    header_variables_test.cpp
    read_test.cpp
    test_utils.cpp
    test_utils.hpp
    write_test.cpp
)

target_compile_features(opendxf-tests PRIVATE cxx_std_23)

# tests may use the library's private headers
target_include_directories(opendxf-tests PRIVATE ${PROJECT_SOURCE_DIR}/opendxf/src)

target_compile_definitions(opendxf-tests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

target_link_libraries(opendxf-tests
    PRIVATE
        opendxf
        GTest::gmock_main
)

gtest_discover_tests(opendxf-tests)
