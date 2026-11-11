#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "asm/impl_base/nodes/nodes_macro.h"
#include "asm/parser/parser_mcr.h"
#include "cstd//macro.h"

import parser;
import nodes;
import reader;
import simple_cstd;
import parser_b1;
import parser_b2;
import parser_wrappers_b2;
import parser_utils;
import cp_consts;
import costd;
import compl_flags;
import parser_concepts;
import gstd;

using namespace parser;
using namespace common;
using namespace wrappers;
using namespace nodes::templ;
using namespace cp_consts::traits;

// ReSharper disable once CppUseInternalLinkage

template <typename Base>
struct TestMxn : Base {
    using Base::Base;
};

namespace {
    struct ClassN1 : ClearClassN<ParseFlags{.useWrite = false, // NOLINT(*-pro-type-member-init)
                                            .parseCpoolStructure = true,
                                            .handleModeCpool = HandleMode::FULL,
                                            .handleModeIfaces = HandleMode::FULL,
                                            .handleModeFields = HandleMode::FULL,
                                            .handleModeMethods = HandleMode::FULL,
                                            .handleModeAttrs = HandleMode::FULL,
                                            .parseLength = ParseLength::FULL}> {
        def::MakeHasParseSize<JaacVector<cp_var>> cpool{};
        JaacVector<CpoolSizeT> ifaces{};
        JaacVector<MFInfo<>> fields{}, methods{};
        JaacVector<FullAttribute<>> attrs{};
    };
} // namespace

int main() {
    constexpr auto path = reader::PreRWFileData{"D:/you_p_only/cpp_elems/javaAsmAtCpp/test/test.class"};
    if (auto result = parser::parse<ClassN1>(path)) {
        auto [node, flag] = result->to_pair();
        int d = 0;
        // ReadPtrBySpec<cflags::class_file_ptr> r{nullptr, nullptr};
    } else {
        std::cerr << "Test failed: " << static_cast<uint32_t>(result.error()) << std::endl;
    }

    // auto result6 = parser::parse<nodes::full::EmptyClassN<{.useWrite = false}>>(path);
}
