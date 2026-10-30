add_library(opendxf STATIC
    include/opendxf/coordinate.hpp
    include/opendxf/document.hpp
    include/opendxf/entities.hpp
    include/opendxf/header.hpp
    include/opendxf/header_variables.hpp
    include/opendxf/io/error.hpp
    include/opendxf/io/read_stream_base.hpp
    include/opendxf/io/read.hpp
    include/opendxf/io/write.hpp
    include/opendxf/layer.hpp
    include/opendxf/opendxf.hpp
    include/opendxf/properties.hpp
    include/opendxf/tables.hpp
    src/header_variables.cpp
    src/io/parse.hpp
    src/io/read_stream_base.cpp
    src/io/read.cpp
    src/io/reader.cpp
    src/io/reader.hpp
    src/io/transparency.cpp
    src/io/transparency.hpp
    src/io/write.cpp
    src/overload.hpp
)

target_include_directories(opendxf PUBLIC include PRIVATE src)

target_compile_features(opendxf PUBLIC cxx_std_23)
