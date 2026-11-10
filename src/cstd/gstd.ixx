module;

#include <concepts>
#include <cassert>
#include "macro.h"

export module gstd;

import simple_cstd;
import costd;

export namespace gstd {
    // common part
    template <typename T>
    concept is_wid_prim = std::is_fundamental_v<T> || std::is_final_v<T> || std::is_pointer_v<T> ||
                          std::is_reference_v<T> || std::is_enum_v<T> || std::is_array_v<T>;

    // widstruct part

    template <typename T>
    CPP_KOSTYL struct GetWidstructDefBaser {
        using Result = T::WidType;
    };

    template <typename T>
    struct prim_unstrict_widstruct {
        template <typename>
        friend struct GetWidstructDefBaser;

    protected:
        T instance;
        CPP_KOSTYL using WidType = T;

    public:
        template <typename Y>
            requires(!std::same_as<Y, const T>) && cstd::is_implicit_convertable<const T, Y>
        /*implicit*/ constexpr operator Y() const noexcept {
            return instance;
        }

        template <typename Y>
            requires(!cstd::is_implicit_convertable<const T, Y>) && std::constructible_from<Y, const T>
        explicit constexpr operator Y() const noexcept {
            return instance;
        }

        template <typename Y>
            requires(!std::same_as<Y, T>) && cstd::is_implicit_convertable<T, Y>
        /*implicit*/ constexpr operator Y() noexcept {
            return instance;
        }

        template <typename Y>
            requires(!cstd::is_implicit_convertable<T, Y>) && std::constructible_from<Y, T>
        explicit constexpr operator Y() noexcept {
            return instance;
        }

        template <typename Y>
        /*implicit*/ constexpr prim_unstrict_widstruct(Y any)
            requires(!std::same_as<Y, T>) && cstd::is_implicit_convertable<Y, T>
            : instance(any) {
        }

        constexpr prim_unstrict_widstruct()
            requires cstd::is_implicit_default_constructable<T>
            : instance(T{}) {
        }

        explicit constexpr prim_unstrict_widstruct()
            requires std::is_default_constructible_v<T> && (!cstd::is_implicit_default_constructable<T>)
            : instance(T{}) {
        }

        template <typename... Args>
        explicit constexpr prim_unstrict_widstruct(Args... args)
            requires(sizeof...(Args) > 1 || (sizeof...(Args) == 1 && !cstd::is_implicit_convertable<Args...[0], T>) ) &&
                    std::constructible_from<T, Args...>
            : instance(T{args...}) {
        }

        /*implicit*/ constexpr prim_unstrict_widstruct(T t) noexcept
            requires(!cstd::larger_t_ptr<T>)
            : instance(t) {
        }
        /*implicit*/ constexpr prim_unstrict_widstruct(T& t) noexcept
            requires cstd::larger_t_ptr<T>
            : instance(t) {
        }
        /*implicit*/ constexpr prim_unstrict_widstruct(T&& t) noexcept
            requires cstd::larger_t_ptr<T>
            : instance(std::move(t)) {
        }
    };

    template <typename T>
    struct prim_strict_widstruct : prim_unstrict_widstruct<T> {
        using prim_unstrict_widstruct<T>::prim_unstrict_widstruct;

        /*implicit*/ constexpr prim_strict_widstruct(T inst) = delete;
        /*implicit*/ constexpr prim_strict_widstruct(prim_unstrict_widstruct<T> inst) = delete;
    };

    // struct version

    template <typename>
    struct struct_unstrict_widstruct {};

    template <typename T>
        requires(!is_wid_prim<T>)
    struct struct_unstrict_widstruct<T> : T {
        template <typename>
        friend struct GetWidstructDefBaser;
        using T::T;

    protected:
        using WidType = T;
    };

    template <typename T>
    struct struct_strict_widstruct : T {
        using T::T;

        /*implicit*/ constexpr struct_strict_widstruct(T inst) = delete;
        /*implicit*/ constexpr struct_strict_widstruct(struct_unstrict_widstruct<T> inst) = delete;
    };

    template <typename T>
    using unstrict_widstruct =
            std::conditional_t<is_wid_prim<T>, prim_unstrict_widstruct<T>, struct_unstrict_widstruct<T>>;

    template <typename T>
    using strict_widstruct = std::conditional_t<is_wid_prim<T>, prim_strict_widstruct<T>, struct_strict_widstruct<T>>;

    template <typename T, bool strict = false>
    using struct_widstruct = std::conditional_t<strict, struct_strict_widstruct<T>, struct_unstrict_widstruct<T>>;

    template <typename T, bool strict = false>
    using prim_widstruct = std::conditional_t<strict, prim_strict_widstruct<T>, prim_unstrict_widstruct<T>>;

    template <typename T, bool strict = false>
    using widstruct = std::conditional_t<strict, strict_widstruct<T>, unstrict_widstruct<T>>;

#if 0
    template <typename, typename>
    struct WidstructTraits {
        constexpr static bool hasResult = false;
    };

    template <typename T>
    struct WidstructTraits<struct_strict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = true;
        constexpr static bool prim = false;
        using WidenedT = T;
    };

    template <typename T>
    struct WidstructTraits<struct_unstrict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = false;
        constexpr static bool prim = false;
        using WidenedT = T;
    };

    template <typename T>
    struct WidstructTraits<prim_strict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = true;
        constexpr static bool prim = true;
        using WidenedT = T;
    };

    template <typename T>
    struct WidstructTraits<prim_unstrict_widstruct<T>, T> {
        constexpr static bool hasResult = true;
        constexpr static bool strict = false;
        constexpr static bool prim = true;
        using WidenedT = T;
    };

    template <typename DirT, typename TB = GetWidstructDefBaser<std::remove_cvref_t<DirT>>::Result>
    using MakeWidstructTraits = WidstructTraits<std::remove_cvref_t<DirT>, TB>;
#endif

    template <typename DirT, typename TB = GetWidstructDefBaser<std::remove_cvref_t<DirT>>::Result>
        requires(!cstd::larger_t_ptr<std::remove_cv_t<DirT>>)
    CPP_KOSTYL constexpr decltype(auto) unwrap(DirT widst) noexcept {
        return static_cast<TB>(widst);
    }

    template <typename DirT, typename TB = GetWidstructDefBaser<std::remove_cvref_t<DirT>>::Result>
        requires cstd::larger_t_ptr<std::remove_cv_t<DirT>>
    CPP_KOSTYL constexpr decltype(auto) unwrap(DirT& widst) noexcept {
        return static_cast<TB>(widst);
    }

    // ccompl value part
    template <typename T, T value>
    struct ccompl_value {
        /*implicit*/ constexpr operator T() const noexcept {
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

        constexpr cond_pair(T first, Y second) noexcept : first(first), second(second) {
        }
    };

    template <typename T, typename U, U IU>
    struct cond_pair<T, ccompl_value<U, IU>> : cstd::pair_cast_mxn<T, U> {
        T first;
        static constexpr U second = IU;

        constexpr cond_pair(T first, U sec) noexcept : first(first) {
            assert(sec == second);
        }

        explicit constexpr cond_pair(T first) noexcept : first(first) {
        }
    };
} // namespace gstd
