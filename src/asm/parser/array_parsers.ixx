module;

#include <expected>

#include "parser_mcr.h"

export module array_parsers;

import opcodes;
import parser_utils;
import cp_consts;
import read_utils;
import parser_concepts;
import parser_wrappers_b2;
import parser_b2;
import simple_cstd;

export namespace parser {
    template <typename T, bool skip0 = false>
    struct JaacIterator {
        T& obj;

        auto begin() noexcept {
            if constexpr (skip0) {
                return ++obj.jaac_begin();
            } else {
                return obj.jaac_begin();
            }
        }

        auto end() noexcept {
            return obj.jaac_end();
        }
    };

    template <bool hasParseSize>
    size_t get_array_deser_size(auto&& jaacArray) noexcept {
        if constexpr (hasParseSize) {
            return jaacArray.jaac_get_parse_size();
        } else {
            return jaacArray.jaac_size();
        }
    }

    template <typename OutCowr, HandleMode handleMode, typename ArrayRSizeT = wrappers::ArraySizeT, bool hasParseSize = false, typename SpecR, typename SpecW,
              bool skip0 = hasParseSize, typename ArrDirT>
    void deser_array(WritePtrBySpec<SpecW>& writePtr, ArrDirT&& jaacArray, auto&& deserializer1) noexcept {
        using ArrT = std::remove_cvref_t<ArrDirT>;

        DEF_BOOL_FLAG_AT_GEN(isJaacArrRawDeserable, false, decltype(jaacArray));
        static_assert(!isJaacArrRawDeserableFlag || JaacDeserable<ArrT, SpecW>,
                      "Invalid raw deserializable jaac array type (with flag rawDeserable = true). Expected requires "
                      "JaacDeserable<TYPE, SPEC_W> (from asm.parser_utils)");

        if constexpr (isJaacArrRawDeserableFlag) {
            jaacArray.deser();
        } else {
            static_assert(handleMode == HandleMode::NONE || JaacIterable<ArrT, OutCowr>,
                          "Bad jaac array (not jaac iterable)");
            static_assert(handleMode == HandleMode::FULL || JaacTranslator<ArrT>);

            if constexpr (handleMode == HandleMode::FULL) {
                ArrayRSizeT{get_array_deser_size<hasParseSize>(jaacArray)}.deser(writePtr);
            } else if constexpr (handleMode == HandleMode::APPEND) {
                ArrayRSizeT{
                    jaacArray.jaac_get_source_data_size() +
                            get_array_deser_size<hasParseSize>(jaacArray)
                }.deser(writePtr);
            } else {
                ArrayRSizeT{jaacArray.jaac_get_source_data_size()}.deser(writePtr);
            }
            if constexpr (handleMode != HandleMode::FULL) {
                writePtr.deser_range(UncheckedReadPtr{jaacArray.jaac_get_source_data_interval_begin(), jaacArray.jaac_get_source_data_interval_end()});
            }

            if constexpr (handleMode != HandleMode::NONE) {
                for (auto& i : JaacIterator<ArrT, skip0>{jaacArray}) {
                    deserializer1(i);
                }
            }
        }
    }

    template <typename SpecR>
    ReadPtrBySpec<SpecR> parse_att_interval(ReadPtrBySpec<SpecR>& readPtr) noexcept {
        auto readPtr2 = readPtr;
        if (!readPtr.template check_memory_and_move<CpoolRSizeT>()) [[unlikely]]
            return {nullptr, nullptr}; // name_idx
        auto length = readPtr.template memcpy_get_rev<AttrsInnerRSizeT>();
        if (!length) [[unlikely]]
            return {nullptr, nullptr};
        if (!readPtr.check_memory_and_move(length)) [[unlikely]]
            return {nullptr, nullptr};
        readPtr2.endPtr = readPtr.i;
        return readPtr2;
    }

    template <bool handle = false, typename SpecR>
    ParseErr parse1_const(ReadPtrBySpec<SpecR>& readPtr, auto&& handler) noexcept {
        return parse1_const_big_ctx<handle>(
                readPtr, [&](opcodes::TagIdEnum, cp_consts::traits::cp_var result) { handler(result); }, [] {});
    }

    template <bool shouldHasCorrectCollection, typename ElemT, typename SpecR>
    bool parse_simple_collection(ReadPtrBySpec<SpecR>& readPtr, auto&& array, const wrappers::ArraySizeT mf_size) {
        for (ArrayRSizeT i = 0; i < static_cast<ArrayRSizeT>(mf_size); ++i) {
            if constexpr (shouldHasCorrectCollection) {
                auto opt_elem = ElemT::parse(readPtr);
                if (!opt_elem) [[unlikely]] {
                    return false;
                }
                DEF_ASSERT_FALSE(array.jaac_emplace_back(std::move(*opt_elem)));
            } else {
                DEF_ASSERT_FALSE(ElemT::check_and_move(readPtr));
            }
        }
        return true;
    }

    template <bool shouldHasCorrectCollection, typename SpecR>
    bool parse_MF(ReadPtrBySpec<SpecR>& readPtr, auto&& array, wrappers::ArraySizeT mf_size) {
        return parse_simple_collection<shouldHasCorrectCollection, common::MFInfo<>, SpecR>(readPtr, array, mf_size);
    }
} // namespace parser
