cmake_minimum_required(VERSION 4.3.4)

project(opendxf LANGUAGES CXX)

option(OPENDXF_BUILD_TESTS "Build the opendxf tests" ${PROJECT_IS_TOP_LEVEL})
option(OPENDXF_BUILD_EXAMPLES "Build the opendxf examples" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(opendxf)

if(OPENDXF_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(OPENDXF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
