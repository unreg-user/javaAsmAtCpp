module;

#include <concepts>
#include <optional>
#include <vector>

export module parser_concepts;

import const_i;
import parser_utils;
import simple_cstd;
import read_utils;
import parser_b1;

export namespace parser {
    template <typename T, typename SpecR, typename SpecW>
    concept JaacParsable = requires(T t, FileReadPtrBySpec<SpecR> readPtr, FileWritePtrBySpec<SpecW> writePtr) {
        { T::parse(readPtr) } -> std::same_as<std::optional<T>>;
        { T::check_and_move(readPtr) } -> std::same_as<bool>;
        { t.deser(writePtr) } -> std::same_as<bool>;
    };

    template <typename T>
    concept JaacClassNT = requires(T t) {
        { T::parseLength } -> std::assignable_from<const bool>;
    } && std::is_default_constructible_v<T>;

    template <typename T, typename InputElT, typename OutputElT>
    concept JaacCollection = requires(T t, size_t iSize, InputElT inEl) {
        { t.jaac_reserve(iSize) } -> std::same_as<void>;
        { t.jaac_push_back(inEl) } -> std::same_as<bool>;
        { t.jaac_size() } -> std::same_as<size_t>;
        { t.jaac_begin() } -> std::input_iterator;
        { t.jaac_begin() } -> std::output_iterator<OutputElT>;
        // ReSharper disable once CppRedundantParentheses
        { t.jaac_end() } -> std::sentinel_for<decltype((t.jaac_begin()))>;

        // optional:
        //  { t.jaac_start_handle(node) } -> std::same_as<void>;
        //  { t.jaac_postcomplete_handle(node) } -> std::same_as<void>;
        //  { t.jaac_final_handle(node) } -> std::same_as<void>;
    } && std::is_default_constructible_v<T>;

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
}