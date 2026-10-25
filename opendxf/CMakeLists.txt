add_library(opendxf STATIC
    include/opendxf/coordinate.hpp
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

target_compile_features(opendxf PUBLIC cxx_std_23)
