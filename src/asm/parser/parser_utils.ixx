module;

#include <cstdint>
#include <optional>

export module parser_utils;

import reader;

export namespace parser {
    template <typename Spec>
    using ReadPtrBySpec = reader::ReadPtr<std::uint8_t, Spec>;

    template <typename Spec>
    using WritePtrBySpec = reader::WriteVecPtr<std::uint8_t, Spec>;

    enum class ParseErr : uint8_t { NONE, UNOPEN, INVALID, POST_PARSE_ERR };

    enum class DeserErr : uint8_t { NONE, UNOPEN };

    enum class HandleMode : uint8_t { NONE, APPEND, FULL };

    enum class ParseLength : uint8_t { VERSION = 0, CPOOL = 1, IFACES = 2, FIELDS = 3, METHODS = 4, FULL = 5 };

    using ArraySizeT = uint16_t;
    using CpoolSizeT = ArraySizeT;
    using IfacesSizeT = uint16_t;
    using AccessFlagsT = uint16_t;
    using AttrSizeT = uint32_t;
} // namespace parser
