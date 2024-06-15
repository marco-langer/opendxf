add_executable(opendxf-examples main.cpp)

target_link_libraries(opendxf-examples PRIVATE opendxf)

target_compile_features(opendxf-examples PRIVATE cxx_std_20)
