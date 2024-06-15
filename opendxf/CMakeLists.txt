add_library(opendxf STATIC
    include/opendxf/document.hpp
    include/opendxf/entities.hpp
    include/opendxf/header.hpp
    include/opendxf/opendxf.hpp
    include/opendxf/tables.hpp
    include/opendxf/write.hpp
    src/write.cpp
)

target_include_directories(opendxf PUBLIC include)

target_compile_features(opendxf PRIVATE cxx_std_20)
