#include "cstd//macro.h"
#include <iostream>

import parser;
import nodes;
import reader;
import simple_cstd;

struct Version {
    uint16_t minor;
    uint16_t major;
};

struct ClassN {
    static constexpr bool useWrite = false;

    Version version{};

    Version m_handle_version(Version ver) {
        ver.major = 0xFFFF;
        return version = ver;
    }
};

int main() {
    constexpr auto path = reader::PreRWFileData{"D:/you_p_only/cpp_elems/javaAsmAtCpp/test/test.class"};
    if (auto result = parser::parse<ClassN>(path)) {
        auto [node, flag] = *result;
        int d = 0;
    } else {
        std::cerr << "Test failed: " << static_cast<uint32_t>(result.error()) << std::endl;
    }
}
