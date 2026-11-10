module;

#include <cstdint>
#include <optional>
#include <span>

export module parser_utils;

import reader;

export namespace parser {
    template <typename Spec>
    using ReadPtrBySpec = reader::ReadPtr<std::uint8_t, Spec>;

    template <typename Spec>
    using WritePtrBySpec = reader::WriteVecPtr<std::uint8_t, Spec>;

    using UncheckedReadPtr = ReadPtrBySpec<reader::UncheckedReadPtrFlagsStruct>;

    enum class ParseErr : uint8_t { NONE, UNOPEN, INVALID, POST_PARSE_ERR };

    enum class DeserErr : uint8_t { NONE, UNOPEN };

    enum class HandleMode : uint8_t { NONE, APPEND, FULL };

    enum class ParseLength : uint8_t { VERSION = 0, CPOOL = 1, IFACES = 2, FIELDS = 3, METHODS = 4, FULL = 5 };

    using ArrayRSizeT = uint16_t;
    using CpoolRSizeT = ArrayRSizeT;
    using IfacesRSizeT = uint16_t;
    using RawAccessFlagsT = uint16_t;
    using AttrsRSizeT = uint32_t;

    using u8_ptr = uint8_t*;
    using u8_span = std::span<uint8_t>;
} // namespace parser
