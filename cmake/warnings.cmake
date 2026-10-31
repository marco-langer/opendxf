# Compiler warnings for opendxf's own targets. Targets link this PRIVATE,
# so projects that use the library don't get these flags.
#
# The goal is as many warnings as possible. Warnings that currently trigger
# are commented out or disabled with -Wno- until the code is fixed.
add_library(opendxf_warnings INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(opendxf_warnings
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            # designated initializers in tests, examples and the library
            # leave out members without a default member initializer
            -Wno-missing-field-initializers
            # write_coordinate() in src/io/write.cpp is unused
            -Wno-unused-function
            -Wcast-align
            -Wcast-qual
            -Wconversion
            -Wdouble-promotion
            -Wextra-semi
            -Wfloat-equal
            -Wformat=2
            -Wimplicit-fallthrough
            # free functions in the examples lack a declaration
            # -Wmissing-declarations
            -Wnon-virtual-dtor
            -Wnull-dereference
            -Wold-style-cast
            -Woverloaded-virtual
            # variables named like their type, e.g. `const entity& entity`
            # -Wshadow
            -Wsign-conversion
            -Wsign-promo
            -Wundef
            -Wunused
            -Wunused-macros
            -Wzero-as-null-pointer-constant
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(opendxf_warnings
        INTERFACE
            -Wcomma-subscript
            -Wctor-dtor-privacy
            -Wdeprecated-copy-dtor
            -Wduplicated-branches
            -Wduplicated-cond
            -Wlogical-op
            -Wmismatched-tags
            -Wnoexcept
            -Wredundant-decls
            -Wredundant-tags
            -Wstrict-null-sentinel
            -Wsuggest-attribute=noreturn
            -Wsuggest-override
            -Wuseless-cast
            -Wvolatile
    )
endif()

if(MSVC)
    target_compile_options(opendxf_warnings INTERFACE /W4)
endif()
