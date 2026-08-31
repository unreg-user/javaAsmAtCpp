cmake_minimum_required(VERSION 4.2)
project(javaAsmAtCpp)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

if(CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo)$")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif ()

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")

add_executable(javaAsmAtCpp src/main.cpp
)

target_sources(javaAsmAtCpp
        PRIVATE
        src/asm/parser_mcr.h
        src/asm/const_i_mcr.h
        src/asm/insns_mcr.h
        src/asm/opcodes_mcr.h

        PUBLIC
        src/main.cpp
        src/cstd/macro.h

        PUBLIC FILE_SET CXX_MODULES FILES
        src/asm/opcodes.ixx
        src/asm/nodes.ixx
        src/asm/insns.ixx
        src/cstd/cstd_variant.ixx
        src/cstd/cached_2linked_list.ixx
        src/cstd/simple_cstd.ixx
        src/cstd/dstd.ixx
        src/asm/const_i.ixx
        src/asm/parser.ixx
        src/cstd/reader.ixx
        src/compl_flags.ixx
        src/asm/parser_utils.ixx
)
