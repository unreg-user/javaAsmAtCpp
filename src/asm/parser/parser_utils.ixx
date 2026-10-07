module;

#include <cstdint>
#include <optional>

export module parser_utils;

import reader;

export namespace parser {
    template <typename Spec>
    using FileReadPtrBySpec = reader::ReadPtr<std::uint8_t, Spec>;

    template <typename Spec>
    using FileWritePtrBySpec = reader::WriteVecPtr<std::uint8_t, Spec>;

    enum class ParseErr : uint8_t { NONE, UNREAD, INVALID };

    enum class ParseResultFlags : uint8_t { NONE, UNWRITE };

    enum class HandleMode : uint8_t { NONE, APPEND, FULL };

    enum class ParseLength : uint8_t { VERSION = 0, CPOOL = 1, IFACES = 2, FIELDS = 3, METHODS = 4, FULL = 5 };

    using ArraySizeT = uint16_t;
    using CPoolSizeT = ArraySizeT;
    using IfacesSizeT = uint16_t;
    using AccessFlagsDefT = uint16_t;
    using AttSizeT = uint32_t;
} // namespace parser
