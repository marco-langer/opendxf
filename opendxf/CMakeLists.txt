find_package(fmt CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

message(STATUS "Using fmt v.${fmt_VERSION}")
message(STATUS "Using tl-expected v.${tl-expected_VERSION}")

add_library(opendxf STATIC
    include/opendxf/document.hpp
    include/opendxf/entities.hpp
    include/opendxf/error.hpp
    include/opendxf/header.hpp
    include/opendxf/ireadstream.hpp
    include/opendxf/opendxf.hpp
    include/opendxf/read.hpp
    include/opendxf/tables.hpp
    include/opendxf/write.hpp
    src/ireadstream.cpp
    src/read.cpp
    src/reader.cpp
    src/reader.hpp
    src/write.cpp
)

target_include_directories(opendxf PUBLIC include)

target_compile_features(opendxf PRIVATE cxx_std_20)

target_link_libraries(opendxf
    PUBLIC
        tl::expected
    PRIVATE
        fmt::fmt
)
