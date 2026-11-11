module;

#include <concepts>
#include <optional>
#include <vector>

#include "../../cstd/costd_mcr.h"
#include "parser_mcr.h"

export module parser_concepts;

import parser_utils;
import read_utils;
import parser_b1;
import dstd;
import gstd;
import costd;
import parser_wrappers_b2;
import simple_cstd;

export namespace parser {
    template <typename ClassNT>
    concept useWriteConcept = DEF_BOOL_CNT_FLAG(useWrite);

    template <typename T, typename SpecR>
    concept JaacParsable = requires(T t, ReadPtrBySpec<SpecR>& readPtr) {
        { T::parse(readPtr) } -> cstd::is_opt_t<cstd::SameAsCowr<T&>>;
        { T::check_and_move(readPtr) } -> std::same_as<bool>;
    };

    template <typename T, typename SpecW>
    concept JaacDeserable = requires(T t, WritePtrBySpec<SpecW>& writePtr) {
        { t.deser(writePtr) } -> gstd::bool_or_true;
    };

    template <typename T, typename JaacCollection, typename SpecW>
    concept JaacInCollectionDeserable =
            requires(T t, size_t idx, JaacCollection& collection, WritePtrBySpec<SpecW>& writePtr) {
                { t.deser(writePtr, collection, idx) } -> gstd::bool_or_true;
            };

    template <typename T, typename SpecR, typename SpecW>
    concept JaacSerDeserable = JaacParsable<T, SpecR> && JaacDeserable<T, SpecW>;

    template <typename T>
    concept JaacClassNT =
            REQ_REQ(std::same_as<decltype(T::parseLength), const ParseLength>) && std::is_default_constructible_v<T>;
    // optional:
    // { t.postread_handle() } -> std::same_as<void>;

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
    concept JaacIterable = requires(T t) {
        { t.jaac_begin() } -> std::input_iterator;
        { t.jaac_begin() } -> cstd::anyref_std_with_iter_type<IterElCowr>;
        // ReSharper disable once CppRedundantParentheses
        { t.jaac_end() } -> std::sentinel_for<decltype((t.jaac_begin()))>;
    };

    template <typename T, typename AddingElT>
    concept JaacCollection = requires(T t, size_t iSize, AddingElT inEl) {
        { t.jaac_reserve(iSize) } -> std::same_as<void>;
        { t.jaac_push_back(inEl) } -> gstd::bool_or_true;
        // ReSharper disable once CppFunctionDoesntReturnValue
        { t.jaac_emplace_back(cstd::emplacer{[] () -> AddingElT& { }}) } -> gstd::bool_or_true;
        { t.jaac_size() } -> std::same_as<size_t>;
        // optional:
        // { t.jaac_start_handle(node) } -> std::same_as<void>;
        // { t.jaac_postcomplete_handle(node) } -> std::same_as<void>;
        // { t.jaac_prewrite_handle(node) } -> std::same_as<void>;
    } && std::is_default_constructible_v<T>;

    template <typename T>
    concept JaacTranslator = requires(T t, u8_ptr pos, size_t iSize) {
        { t.jaac_set_source_data_interval_begin(pos) } -> std::same_as<void>;
        { t.jaac_set_source_data_interval_end(pos) } -> std::same_as<void>;
        { t.jaac_set_source_data_size(iSize) } -> std::same_as<void>;
        { t.jaac_get_source_data_interval_begin() } -> std::same_as<u8_ptr>;
        { t.jaac_get_source_data_interval_end() } -> std::same_as<u8_ptr>;
        { t.jaac_get_source_data_size() } -> std::same_as<size_t>;
    } && std::is_default_constructible_v<T>;

    template <typename T>
    concept WithParseSize = requires(T t, size_t iSize) {
        { t.jaac_set_parse_size(iSize) } -> std::same_as<void>;
        { t.jaac_get_parse_size() } -> std::same_as<size_t>;
    };

    template <typename SpecW>
    struct JaacDeserableCowr {
        template <typename T>
        COWR_INST = JaacDeserable<T, SpecW>;
    };

    // Cowrs
    struct FieldInfoCowr {
        template <typename T, typename AttrSpanCowr>
        COWR_INST = requires(T t) {
            requires cstd::is_implicit_convertable<decltype(t.accessFlags), wrappers::AccessFlags>;
            requires cstd::is_implicit_convertable<decltype(t.nameIdx), wrappers::CpoolSizeT>;
            requires cstd::is_implicit_convertable<decltype(t.descriptorIdx), wrappers::CpoolSizeT>;
            requires AttrSpanCowr::COWR_USE<decltype(t.attrs)>;
        };
    };
} // namespace parser
