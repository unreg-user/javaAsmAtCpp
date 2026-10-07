module;

#include <optional>
#include <tuple>
#include "../../cstd/macro.h"

export module read_utils;

import parser_utils;

export namespace reader::utils {
    template <typename Self, typename... Args, typename SpecR>
    F_INLINE NODISCARD std::optional<Self> parse_rev(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
        auto opt_values = std::tuple{dataPtr.template parse_rev<Args>()...};

        if (std::apply([](auto&&... values) { return (!values.has_value() || ...); }, opt_values)) [[unlikely]] {
            return std::nullopt;
        }

        return std::make_optional(
                std::apply([](auto&&... args) { return Self{(*args)...}; }, opt_values)); // I hate this ecosystem...
    }

    template <typename... Args, typename SpecR>
    F_INLINE NODISCARD bool check_and_move(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
        return (... && dataPtr.template check_and_move<Args>());
    }

    template <typename SpecW>
    F_INLINE NODISCARD bool deser_rev(parser::FileWritePtrBySpec<SpecW>& dataPtr, auto... args) noexcept {
        return (... && dataPtr.deser_rev(args));
    }

    template <typename SpecW>
    F_INLINE NODISCARD bool deser_raw(parser::FileWritePtrBySpec<SpecW>& dataPtr, auto... args) noexcept {
        return dataPtr.deser(std::tuple{args...});
    }
} // namespace reader::utils
