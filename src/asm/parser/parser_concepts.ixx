module;

#include <concepts>
#include <optional>
#include <vector>

#include "parser_mcr.h"
#include "../../cstd/costd_mcr.h"

export module parser_concepts;

import parser_utils;
import read_utils;
import parser_b1;
import dstd;
import gstd;
import costd;

export namespace parser {
    template <typename ClassNT>
    concept useWriteConcept = DEF_BOOL_CNT_FLAG_RET(useWrite);
    
    template <typename T, typename SpecR>
    concept JaacParsable = requires (T t, ReadPtrBySpec<SpecR> readPtr) {
        { T::parse(readPtr) } -> std::same_as<std::optional<T>>;
        { T::check_and_move(readPtr) } -> std::same_as<bool>;
    };

    template <typename T, typename SpecW>
    concept JaacDeserable = requires (T t, WritePtrBySpec<SpecW> writePtr) {
        { t.deser(writePtr) } -> gstd::bool_or_true;
    };

    template <typename T, typename SpecR, typename SpecW>
    concept JaacSerDeserable = JaacParsable<T, SpecR> && JaacDeserable<T, SpecW>;

    template <typename T>
    concept JaacClassNT = requires(T t) {
        { t.postread_handle() } -> std::same_as<void>;
        { T::parseLength } -> std::assignable_from<const bool>;
    } && std::is_default_constructible_v<T>;

    template <typename ClassNT>
    // ReSharper disable once CppRedundantParentheses
    using JaacClassNTRetConverted = TYPE_WRAPPER_LAMBDA({
        if constexpr (requires { std::declval<ClassNT>().jaac_return_convert(); }) {
            return dstd::args::TypeWrapper<decltype((std::declval<ClassNT>().jaac_return_convert()))>{};
        } else {
            return dstd::args::TypeWrapper<ClassNT>{};
        }
    });

    template <typename T, typename IterElCowr>
    concept JaacItrable = requires(T t) {
        { t.jaac_begin() } -> std::input_iterator;
        { t.jaac_begin() } -> cstd::anyref_std_with_iter_type<IterElCowr>;
        // ReSharper disable once CppRedundantParentheses
        { t.jaac_end() } -> std::sentinel_for<decltype((t.jaac_begin()))>;
    };

    template <typename T, typename AddingElT, typename IterateElT>
    concept JaacCollection = requires(T t, size_t iSize, AddingElT inEl) {
        { t.jaac_reserve(iSize) } -> std::same_as<void>;
        { t.jaac_push_back(inEl) } -> gstd::bool_or_true;
        { t.jaac_size() } -> std::same_as<size_t>;
        // optional:
        //  { t.jaac_start_handle(node) } -> std::same_as<void>;
        //  { t.jaac_postcomplete_handle(node) } -> std::same_as<void>;
        //  { t.jaac_final_handle(node) } -> std::same_as<void>;
    } && std::is_default_constructible_v<T> && JaacItrable<T, IterateElT>;

    template <typename T>
    concept JaacTranslator = requires(T t, uint8_t* pos, size_t iSize) {
        { t.jaac_set_source_data_interval_first(pos) } -> std::same_as<void>;
        { t.jaac_set_source_data_interval_last(pos) } -> std::same_as<void>;
        { t.jaac_set_source_data_size(iSize) } -> std::same_as<void>;
        { t.jaac_get_source_data_interval_first() } -> std::same_as<uint8_t*>;
        { t.jaac_get_source_data_interval_last() } -> std::same_as<uint8_t*>;
        { t.jaac_get_source_data_size() } -> std::same_as<size_t>;
    } && std::is_default_constructible_v<T>;

    template <typename T>
    concept WithParseSize = requires(T t, size_t iSize) {
        { t.jaac_set_parse_size(iSize) } -> std::same_as<void>;
        { t.jaac_get_parse_size() } -> std::same_as<size_t>;
    };
} // namespace parser
