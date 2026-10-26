module;

#include <concepts>

#include "macro.h"

export module gstd;

import simple_cstd;

export namespace gstd {
    // common part
    template <typename T>
    concept is_wid_prim = std::is_fundamental_v<T> || std::is_final_v<T>;

    // widstruct part
    template <typename T>
    struct getWidstructDefBaser;

    template <typename T>
    struct prim_unstrict_widstruct {
        friend getWidstructDefBaser;
    protected:
        T instance;
        using WidType = T;
    public:
        template <typename Y> requires !std::same_as<Y, const T> && cstd::is_implicit_convertable<const T, Y>
        /*implicit*/ operator Y() const noexcept {
            return instance;
        }

        template <typename Y> requires (!cstd::is_implicit_convertable<const T, Y>) && std::constructible_from<Y, const T>
        explicit operator Y() const noexcept {
            return instance;
        }

        template <typename Y> requires !std::same_as<Y, T> && cstd::is_implicit_convertable<T, Y>
        /*implicit*/ operator Y() noexcept {
            return instance;
        }

        /*implicit*/ operator T&() & {
            return instance;
        }

        template <typename Y> requires (!cstd::is_implicit_convertable<T, Y>) && std::constructible_from<Y, T>
        explicit operator Y() noexcept {
            return instance;
        }

        template <typename Y>
        /*implicit*/ prim_unstrict_widstruct(Y any) requires cstd::is_implicit_convertable<Y, T> : instance(any) {}

        /*implicit*/ prim_unstrict_widstruct() requires cstd::is_implicit_default_constructable<T> : instance(T{}) {}

        explicit prim_unstrict_widstruct() requires std::is_default_constructible_v<T> && !cstd::is_implicit_default_constructable<T> : instance(T{}) {}

        template <typename... Args>
        explicit prim_unstrict_widstruct(Args... args) requires (sizeof...(Args) > 1 || (sizeof...(Args) == 1 && !cstd::is_implicit_convertable<Args...[0], T>)) && std::constructible_from<T, Args...> : instance(T{args...}) {}
    };

    template <typename T>
    struct prim_strict_widstruct : prim_unstrict_widstruct<T> {
        using prim_unstrict_widstruct<T>::prim_unstrict_widstruct;

        /*implicit*/ prim_strict_widstruct(T inst) = delete;
        /*implicit*/ prim_strict_widstruct(prim_unstrict_widstruct<T> inst) = delete;
    };

    template <typename T>
    struct struct_unstrict_widstruct : T {
        friend getWidstructDefBaser;
        using T::T;
    protected:
        using WidType = T;
    };

    template <typename T>
    struct struct_strict_widstruct : T {
        using T::T;

        /*implicit*/ struct_strict_widstruct(T inst) = delete;
        /*implicit*/ struct_strict_widstruct(struct_unstrict_widstruct<T> inst) = delete;
    };

    template <typename T>
    using unstrict_widstruct = std::conditional_t<is_wid_prim<T>, prim_unstrict_widstruct<T>, struct_unstrict_widstruct<T>>;

    template <typename T>
    using strict_widstruct = std::conditional_t<is_wid_prim<T>, prim_strict_widstruct<T>, struct_strict_widstruct<T>>;

    template <typename T, bool strict = false>
    using struct_widstruct = std::conditional_t<strict, struct_strict_widstruct<T>, struct_unstrict_widstruct<T>>;

    template <typename T, bool strict = false>
    using prim_widstruct = std::conditional_t<strict, prim_strict_widstruct<T>, prim_unstrict_widstruct<T>>;

    template <typename T, bool strict = false>
    using widstruct = std::conditional_t<strict, strict_widstruct<T>, unstrict_widstruct<T>>;

    template <typename, typename>
    struct widstructTraits {
        constexpr static bool hasResult = false;
    };

    template <typename T>
    struct widstructTraits<struct_strict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = true;
        constexpr static bool prim = false;
        using WidenedT = T;
    };

    template <typename T>
    struct widstructTraits<struct_unstrict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = false;
        constexpr static bool prim = false;
        using WidenedT = T;
    };

    template <typename T>
    struct widstructTraits<prim_strict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = true;
        constexpr static bool prim = true;
        using WidenedT = T;
    };

    template <typename T>
    struct widstructTraits<prim_unstrict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = false;
        constexpr static bool prim = true;
        using WidenedT = T;
    };

    template <typename DirT, typename TB = getWidstructDefBaser<std::remove_cvref_t<DirT>>::Result>
    using makeWidsructTraits = widstructTraits<std::remove_cvref_t<DirT>, TB>;

    template <typename T>
    struct getWidstructDefBaser<T> {
        using Result = T::WidType;
    };

    template <typename DirT, typename TB = getWidstructDefBaser<std::remove_cvref_t<DirT>>::Result> requires !cstd::larger_t_ptr<std::remove_cv_t<DirT>>
    decltype(auto) unwrap(DirT widst) noexcept {
        return static_cast<TB>(widst);
    }

    template <typename DirT, typename TB = getWidstructDefBaser<std::remove_cvref_t<DirT>>::Result> requires cstd::larger_t_ptr<std::remove_cv_t<DirT>>
    decltype(auto) unwrap(DirT& widst) noexcept {
        return static_cast<TB>(widst);
    }

    // ccompl value part
    template <typename T, T value> struct ccompl_value {
        /*implicit*/ operator T() const noexcept {
            return value;
        }
    };

    using true_void = ccompl_value<bool, true>;
    using false_void = ccompl_value<bool, false>;

    template <typename T>
    concept bool_or_true = std::same_as<T, bool> || std::same_as<T, true_void>;

    template <typename T, typename Y>
    struct cond_pair : cstd::pair_cast_mxn<T, Y> {
        T first;
        Y second;

        constexpr cond_pair(T first, Y second) noexcept : first(first), second(second) {}
    };

    template <typename T, typename U, U IU>
    struct cond_pair<T, ccompl_value<U, IU>> : cstd::pair_cast_mxn<T, U> {
        T first;
        static constexpr U second = IU;

        constexpr cond_pair(T first, U sec) noexcept : first(first) {
            assert(sec == second);
        }

        explicit constexpr cond_pair(T first) noexcept : first(first) {}
    };
}