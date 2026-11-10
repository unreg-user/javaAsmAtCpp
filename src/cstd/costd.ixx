module;

#include <functional>
#include <optional>
#include <type_traits>

#include "costd_mcr.h"
#include "macro.h"

export module costd;

import simple_cstd;

export namespace cstd {
    template <typename T, K1 Y>
    constexpr bool is_cont_in = false;

    template <typename T, K1 Y>
    constexpr bool is_cont_in<Y<T>, Y> = true;

    template <typename...>
    constexpr bool static_assert_false = false;

    template <typename From, typename To>
    concept is_implicit_convertable = std::is_convertible_v<From, To>;

    template <typename From, typename To>
    concept is_ie_covertable = requires (From from) { static_cast<To>(from); };

    template <typename T>
    concept is_implicit_default_constructable = requires { [] () -> T { return {}; }; };

    template <typename T, typename DerefReturnsCowr>
    concept is_opt_t = requires (T t) { requires DerefReturnsCowr::COWR_USE<decltype(*t)>; } && is_ie_covertable<T, bool>; // && is_implicit_convertable<T, std::optional<YCowr>>

    template <typename Y>
    struct SameAsCowr {
        template <typename T>
        COWR_INST = std::same_as<T, Y>;
    };

    template <typename Y>
    struct IsAssignableCowr {
        template <typename T>
        COWR_INST = std::is_assignable_v<T, Y>;
    };

    template <typename Y>
    struct ImplConvertableCowr {
        template <typename T>
        COWR_INST = is_implicit_convertable<T, Y>;
    };

    template <typename Y>
    struct IEConvertableCowr {
        template <typename T>
        COWR_INST = is_ie_covertable<T, Y>;
    };

    template <typename YCowr>
    struct IsOptCowr {
        template <typename T>
        COWR_INST = is_opt_t<T, YCowr>;
    };

    template <typename T, typename ICowr>
    concept anyref_std_with_iter_type = ICowr::COWR_USE<std::iter_value_t<std::remove_cvref_t<T>>>;
}