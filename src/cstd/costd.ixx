module;

#include <functional>
#include <type_traits>
#include <xtr1common>

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

    template <typename T>
    concept is_implicit_default_constructable = requires { [] () -> T { return {}; }; };

    struct SameAsCowr {
        template <typename T, typename Y>
        CONCEPT Result = std::same_as<T, Y>;
    };

    struct ImplConvertableCowr {
        template <typename T, typename Y>
        CONCEPT Result = is_implicit_convertable<T, Y>;
    };

    template <typename T, typename ICowr>
    concept anyref_std_with_iter_type = ICowr::template Result<std::iter_value_t<std::remove_cvref_t<T>>>;
}