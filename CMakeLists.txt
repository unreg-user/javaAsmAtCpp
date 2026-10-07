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
        src/asm/parser/base_classes/parser_b2.ixx
)

target_sources(javaAsmAtCpp
        PUBLIC
        src/asm/parser/parser_mcr.h
        src/asm/impl_base/cp_consts_mcr.h
        src/asm/impl_base/insns_mcr.h
        src/asm/opcodes_mcr.h

        PUBLIC
        src/main.cpp
        src/cstd/macro.h
        src/cstd/macro_dstd.h
        src/asm/impl_base/nodes/nodes_macro.h

        PUBLIC FILE_SET CXX_MODULES FILES
        src/cstd/cstd_variant.ixx
        src/cstd/cached_2linked_list.ixx
        src/cstd/simple_cstd.ixx
        src/cstd/dstd.ixx
        src/cstd/reader.ixx
        src/cstd/ptr_function.ixx
        src/asm/opcodes.ixx
        src/asm/impl_base/nodes/nodes.ixx
        src/asm/impl_base/insns.ixx
        src/asm/parser/parser.ixx
        src/asm/impl_base/cp_consts.ixx
        src/asm/parser/parser_utils.ixx
        src/asm/parser/parser_concepts.ixx
        src/asm/parser/array_parsers.ixx
        src/asm/utils/read_utils.ixx
        src/asm/parser/base_classes/parser_b1.ixx
        src/compl_flags.ixx
)
