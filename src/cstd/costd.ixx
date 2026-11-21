module;

#include <functional>
#include <iosfwd>
#include <optional>
#include <type_traits>

#include "costd_mcr.h"
#include "macro.h"

export module costd;

import simple_cstd;

/* Cowr, because [template <...> concept] is stripped-down :) */
export namespace cstd {
    template <typename T, K1 Y>
    constexpr bool is_cont_in = false;

    template <typename T, K1 Y>
    constexpr bool is_cont_in<Y<T>, Y> = true;

    template <typename...>
    constexpr bool static_assert_false = false;

    template <typename From, typename To>
    concept is_implicit_convertable = std::is_convertible_v<From, To>;

    template <typename T, typename... Args> CPP_KOSTYL // the fact that constructors with more than 1 argument are even categorized into explicit/implicit :)
    concept is_implicit_constructable = requires { [](T) {}({std::declval<Args>()...}); };

    template <typename From, typename To>
    concept is_ie_covertable = requires(From from) { static_cast<To>(from); };

    template <typename T>
    concept is_implicit_default_constructable = requires { []() -> T { return {}; }; };

    template <typename T>
    concept is_console_writable = requires(T t, std::ostream& stream) {
        { stream << t } -> std::same_as<std::ostream&>;
    };

    template <typename T, typename DerefReturnsCowr>
    concept is_opt_t = requires(T t) { requires DerefReturnsCowr::COWR_USE<decltype(*t)>; } &&
                       is_ie_covertable<T, bool>; // && is_implicit_convertable<T, std::optional<DerefReturnsCowr>>

    template <typename Y>
    CPP_KOSTYL struct SameAsCowr {
        template <typename T>
        COWR_INST = std::same_as<T, Y>;
    };

    template <typename Y>
    CPP_KOSTYL struct IsAssignableCowr {
        template <typename T>
        COWR_INST = std::is_assignable_v<T, Y>;
    };

    template <typename Y>
    CPP_KOSTYL struct ImplConvertableCowr {
        template <typename T>
        COWR_INST = is_implicit_convertable<T, Y>;
    };

    template <typename Y>
    CPP_KOSTYL struct IEConvertableCowr {
        template <typename T>
        COWR_INST = is_ie_covertable<T, Y>;
    };

    template <typename YCowr>
    CPP_KOSTYL struct IsOptCowr {
        template <typename T>
        COWR_INST = is_opt_t<T, YCowr>;
    };

    template <typename T, typename ICowr>
    concept anyref_std_with_iter_type = ICowr::COWR_USE<std::iter_value_t<std::remove_cvref_t<T>>>;
} // namespace cstd
