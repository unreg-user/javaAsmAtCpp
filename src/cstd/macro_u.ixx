module;

#include <iostream>

export module macro_u;

export namespace macro_internal {
    inline void _dassert(const char* expr, const char* file, const unsigned int line, const char* msg) {
        std::cerr << "Assertion failed: (" << expr << "), file " << file
                  << ", line " << line;
        if (msg && msg[0] != '\0') {
            std::cerr << ", with message: " << msg;
        }
        std::cerr << std::endl;
        std::abort();
    }
}