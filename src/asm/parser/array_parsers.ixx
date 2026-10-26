module;

#include <expected>

#include "parser_mcr.h"

export module array_parsers;

import opcodes;
import parser_utils;
import cp_consts;
import read_utils;

export namespace parser {
    template <bool hasParseSize>
    static size_t get_array_deser_size(auto&& jaacArray) {
        if constexpr (hasParseSize) {
            return jaacArray.jaac_get_size();
        } else {
            return jaacArray.jaac_get_parse_size();
        }
    }

    template <HandleMode handleWritten, bool hasParseSize = false, typename SpecR, typename SpecW, bool skip0 = hasParseSize>
    static void deser_array(WritePtrBySpec<SpecW>& writePtr, auto&& jaacArray, auto&& deserializer1) {
        if constexpr (handleWritten == HandleMode::FULL) {
            writePtr.deser_rev(get_array_deser_size<hasParseSize>(jaacArray));
        } else if constexpr (handleWritten == HandleMode::APPEND) {
            writePtr.deser_rev(jaacArray.jaac_get_source_data_size() + get_array_deser_size<hasParseSize>(jaacArray));
        } else { 
            writePtr.deser_rev(jaacArray.jaac_get_source_data_size());
        }
        if constexpr (handleWritten != HandleMode::FULL) { 
            writePtr.deser_range(ReadPtrBySpec<SpecR>{jaacArray.jaac_get_source_data_first(), jaacArray.jaac_get_source_data_last()});
        }

        auto end = jaacArray.jaac_end();
        auto begin = jaacArray.jaac_begin();
        if constexpr (skip0) ++begin;
        for (auto iter = begin; iter != end; ++iter) {
            deserializer1(*iter);
        }
    }
    
    template <typename SpecR>
    static ReadPtrBySpec<SpecR> parse_att_interval(ReadPtrBySpec<SpecR>& readPtr) {
        auto readPtr2 = readPtr;
        if (!readPtr.template check_memory_and_move<CpoolSizeT>()) [[unlikely]] return {nullptr, nullptr}; // name_idx
        auto length = readPtr.template memcpy_get_rev<AttrSizeT>();
        if (!length) [[unlikely]] return {nullptr, nullptr};
        if (!readPtr.check_memory_and_move(length)) [[unlikely]] return {nullptr, nullptr};
        readPtr2.last = readPtr.i - 1;
        return readPtr2;
    }

    template <bool handle = false, typename SpecR>
    static ParseErr parse1_const_big_ctx(ReadPtrBySpec<SpecR>& readPtr, auto&& handler, auto&& skipHandler) {
        DEF_OR_ELSE_OPT_INVALID(opcodes::TagIdType, tagIdRaw);
        auto tagId = opcodes::as_enum(tagIdRaw);

        const auto handle_lambda = [&readPtr] {
            if constexpr (handle) {
                return [&readPtr]<opcodes::TagIdEnum, typename Trait>() noexcept -> cp_consts::traits::opt_variant {
                    if (auto value = Trait::parse(readPtr)) {
                        return {{*value}};
                    }
                    return {};
                };
            } else {
                return [&readPtr]<opcodes::TagIdEnum, typename Trait>() noexcept -> bool {
                    return Trait::check_and_move(readPtr);
                };
            }
        }();

        auto result = cp_consts::switch_tag_id_enum_2compl<
                std::conditional_t<handle, cp_consts::traits::opt_variant, bool>>(tagId, handle_lambda);

        DEF_ASSERT_INVALID(result);
        DEF_ASSERT_INVALID(*result);

        if constexpr (handle) {
            handler(tagId, **result);
        } else {
            skipHandler(tagId);
        }

        return ParseErr::NONE;
    }

    template <bool handle = false, typename SpecR>
    static ParseErr parse1_const(ReadPtrBySpec<SpecR>& readPtr, auto&& handler) {
        return parse1_const_big_ctx<handle>(readPtr, [&] (opcodes::TagIdEnum, cp_consts::traits::variant result) { handler(result); }, []{});
    }
} // namespace parser
