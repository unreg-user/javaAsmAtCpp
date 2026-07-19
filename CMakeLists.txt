cmake_minimum_required(VERSION 4.2)
project(javaAsmAtCpp)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")

add_executable(javaAsmAtCpp src/main.cpp
        timed/macro_cstd.h
)

target_sources(javaAsmAtCpp
        PRIVATE
        src/main.cpp

        PUBLIC
        src/macro.h

        PUBLIC FILE_SET CXX_MODULES FILES
        src/opcodes.ixx
        src/nodes.ixx
        src/insns.ixx
        src/cstd/cstd_variant.ixx
        src/cstd/cached_2linked_list.ixx
        src/cstd/simple_cstd.ixx
)
