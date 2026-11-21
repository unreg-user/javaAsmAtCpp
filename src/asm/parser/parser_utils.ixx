module;

#include <cstdint>
#include <span>
#include "../../cstd/macro.h"

export module parser_utils;

import reader;

export namespace parser {
    template <typename Spec>
    using ReadPtrBySpec = reader::ReadPtr<std::uint8_t, Spec>;

    template <typename Spec>
    using WritePtrBySpec = reader::WriteVecPtr<std::uint8_t, Spec>;

    using UncheckedReadPtr = ReadPtrBySpec<reader::UncheckedReadPtrFlagsStruct>;

    struct ParseErr {
        enum class At : uint8_t { NONE, MAGIC, VERSION, CPOOL, POST_CPOOL, IFACES, FIELDS, METHODS, ATTRS, POST_PARSE };

        At at;

        GEN_DEF_EQ_OP(ParseErr);
    };

    struct DeserErr {
        enum class At : uint8_t { NONE, CPOOL, IFACES, FIELDS, METHODS, ATTRS };

        At at;

        GEN_DEF_EQ_OP(DeserErr);
    };

    enum class HandleMode : uint8_t { NONE, APPEND, FULL };

    enum class ParseJaacComplexCollMode : uint8_t { FULL_SKIP, SKIP_AND_SAVE_INTERVALS, PARSE };

    enum class ParseLength : uint8_t { VERSION = 0, CPOOL = 1, IFACES = 2, FIELDS = 3, METHODS = 4, FULL = 5 };

    using ArrayRSizeT = uint16_t;
    using CpoolRSizeT = ArrayRSizeT;
    using RawAccessFlagsT = uint16_t;
    using AttrsInnerRSizeT = uint32_t;

    using u8_ptr = uint8_t*;
    using u8_span = std::span<uint8_t>;
} // namespace parser
