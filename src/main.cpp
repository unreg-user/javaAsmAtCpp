#include <iostream>
#include <string>
#include "asm/impl_base/nodes/nodes_macro.h"
#include "asm/parser/parser_mcr.h"
#include "cstd//macro.h"

import parser;
import nodes;
import reader;
import simple_cstd;
import const_i;

struct Version {
    uint16_t minor;
    uint16_t major;
};

struct BadVersion {
    uint32_t minor;
    uint16_t major;
};

struct ClassN {
    C_FLAG_E(useWrite);
    C_FLAG_E(handleVersion);

    // C_FLAG_E(handleWritedCpool);

    nodes::templ::def::Cpool<ClassN> cpool;

    Version version{};

    auto m_handle_version(Version ver) {
        ver.major = 0xFFFF;
        version = ver;
        return [&] { return Version{static_cast<uint16_t>( cpool.size() ), static_cast<uint16_t>( cpool.size()) }; };
    }

    auto handle_major_version(const uint16_t major) {
        version.major = major;
        return [] () -> uint16_t { return 144; };
    }
};
/*
struct ClassN6 {
    C_FLAG_E(handleVersion);

    static uint64_t m_handle_version(const uint32_t ver) {
        std::cout << "s";
        return 0;
    }
};*/

int main() {
    constexpr auto path = reader::PreRWFileData{"D:/you_p_only/cpp_elems/javaAsmAtCpp/test/test.class"};
    //if (auto result = parser::parse<ClassN>(path)) {
    //    auto [node, flag] = *result;
    //    int d = 0;
    //} else {
    //    std::cerr << "Test failed: " << static_cast<uint32_t>(result.error()) << std::endl;
    //}

    auto result6 = parser::parse<nodes::full::ClassN<{.useWrite = true, .handleWritedCpool = false}>>(path);
}
