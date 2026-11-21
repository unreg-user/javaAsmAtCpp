module;

#include <iostream>

#include "macro.h"

export module macro_u;

export namespace macro_internal {
#ifndef NDEBUG
    inline void _dassert(const char* expr, const char* file, const unsigned int line, const char* msg) {
        std::cerr << "Assertion failed: (" << expr << "), file " << file
                  << ", line " << line;
        if (msg && msg[0] != '\0') {
            std::cerr << ", with message: " << msg;
        }
        std::cerr << std::endl;
        std::abort();
    }

    struct assertor_ {
        bool bl{false};

        constexpr assertor_() noexcept = default;

        IMPLICIT constexpr assertor_(const bool bl) noexcept : bl(bl) {
            assert(bl);
        }

        constexpr assertor_(const bool bl, const char* assertionText) noexcept : bl(bl) {
            dassert(bl, assertionText);
        }

        IMPLICIT constexpr operator bool() const noexcept {
            return bl;
        }

        constexpr bool operator!() const noexcept {
            return !bl;
        }
    };
#endif
}