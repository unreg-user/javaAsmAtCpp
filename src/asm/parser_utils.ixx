module;

#include <cstdint>

export module parser_utils;

import reader;

export namespace parser {
    template <typename Spec>
    using FileSymPtrBySpec = reader::ReadPtr<std::uint8_t, Spec>;
}