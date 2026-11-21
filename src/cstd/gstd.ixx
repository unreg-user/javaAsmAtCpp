module;

#include <cassert>
#include <concepts>
#include "macro.h"

export module gstd;

import simple_cstd;
import costd;

export namespace gstd {
    // common part
    template <typename T>
    concept is_wid_prim = std::is_fundamental_v<T> || std::is_final_v<T> || std::is_pointer_v<T> ||
                          std::is_reference_v<T> || std::is_enum_v<T> || std::is_array_v<T>;

    // widstruct part (inherit only publicly)

    namespace internal {
        template <typename>
        CPP_KOSTYL struct GetWidstructDefBaser {
            using Result = void;
        };

        template <typename T>
            requires(requires { typename T::WidType; })
        CPP_KOSTYL struct GetWidstructDefBaser<T> {
            using Result = T::WidType;
        };
    }

    template <typename T>
    using GetWidTypeOrVoid = internal::GetWidstructDefBaser<T>::Result;

    template <typename T>
    concept type_is_widstr = !std::same_as<void, typename internal::GetWidstructDefBaser<T>::Result>;

    template <typename WidstT, typename To>
    concept type_is_widstr_to = std::same_as<To, typename internal::GetWidstructDefBaser<WidstT>::Result>;

    template <typename T>
    struct prim_unstrict_widstruct {
        template <typename>
        friend struct internal::GetWidstructDefBaser;

    protected:
        T instance;
        CPP_KOSTYL using WidType = T;

    public:
        CPP_KOSTYL // because i can't write explicit(!cstd::is_implicit_convertable<T&, Y>), because in IMPLICIT
                   // operator Y, Y not ref
                template <typename Y>
            requires(!std::same_as<T, std::remove_cvref_t<Y>>) && std::constructible_from<Y, T&>
        explicit constexpr operator Y() & noexcept(std::is_nothrow_constructible_v<Y, T&>) {
            return static_cast<Y>(instance);
        }

        template <typename Y>
            requires(!std::same_as<T, std::remove_cvref_t<Y>>) && std::constructible_from<Y, const T&>
        explicit constexpr operator Y() const& noexcept(std::is_nothrow_constructible_v<Y, const T&>) {
            return static_cast<Y>(instance);
        }

        template <typename Y>
            requires(!std::same_as<T, std::remove_cvref_t<Y>>) && std::constructible_from<Y, T&&>
        explicit constexpr operator Y() && noexcept(std::is_nothrow_constructible_v<Y, T&&>) {
            return static_cast<Y>(std::move(instance));
        }

        IMPLICIT constexpr operator T&() & noexcept {
            return instance;
        }

        IMPLICIT constexpr operator const T&() const& noexcept {
            return instance;
        }

        IMPLICIT constexpr operator T&&() && noexcept {
            return std::move(instance);
        }

        template <typename Y>
            requires(!std::same_as<std::remove_cvref_t<Y>, prim_unstrict_widstruct> &&
                     !type_is_widstr_to<std::remove_cvref_t<Y>, T> && std::constructible_from<T, Y &&>)
        explicit(!cstd::is_implicit_convertable<Y&&, T>) constexpr prim_unstrict_widstruct(Y&& y) noexcept(
                std::is_nothrow_constructible_v<T, Y&&>) : instance(std::forward<Y>(y)) {
        }

        explicit(!cstd::is_implicit_default_constructable<T>) constexpr prim_unstrict_widstruct() noexcept(
                std::is_nothrow_default_constructible_v<T>)
            requires std::is_default_constructible_v<T>
            : instance(T()) {
        }

        template <typename... Args>
        explicit(!cstd::is_implicit_constructable<T, Args&&...>) constexpr prim_unstrict_widstruct(
                Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
            requires(sizeof...(Args) > 1 && std::constructible_from<T, Args && ...>)
            : instance(T(std::forward<Args>(args)...)) {
        }
    };

    template <typename T>
    struct prim_strict_widstruct : prim_unstrict_widstruct<T> {
        using prim_unstrict_widstruct<T>::prim_unstrict_widstruct;

        IMPLICIT constexpr prim_strict_widstruct(T inst) = delete;
        IMPLICIT constexpr prim_strict_widstruct(prim_unstrict_widstruct<T> inst) = delete;
    };

    // struct version

    template <typename>
    struct struct_unstrict_widstruct {};

    template <typename T>
        requires(!is_wid_prim<T>)
    struct struct_unstrict_widstruct<T> : T {
        template <typename>
        friend struct internal::GetWidstructDefBaser;
        using T::T;

    protected:
        using WidType = T;
    };

    template <typename T>
    struct struct_strict_widstruct : struct_unstrict_widstruct<T> {
        using struct_unstrict_widstruct<T>::struct_unstrict_widstruct;

        IMPLICIT constexpr struct_strict_widstruct(T inst) = delete;
        IMPLICIT constexpr struct_strict_widstruct(struct_unstrict_widstruct<T> inst) = delete;
    };

    namespace internal::raw_traits {
        template <typename T>
        using unstrict_widstruct =
                std::conditional_t<is_wid_prim<T>, prim_unstrict_widstruct<T>, struct_unstrict_widstruct<T>>;

        template <typename T>
        using strict_widstruct =
                std::conditional_t<is_wid_prim<T>, prim_strict_widstruct<T>, struct_strict_widstruct<T>>;

        template <typename T, bool strict = false>
        using widstruct = std::conditional_t<strict, strict_widstruct<T>, unstrict_widstruct<T>>;

        template <typename T, bool strict = false>
        struct with_args {
            using current_widstruct = widstruct<T, strict>;

            struct any_widstruct : current_widstruct {
                using current_widstruct::current_widstruct;
            };
        };
    } // namespace internal::raw_traits

    template <typename T, bool strict = false>
    using any_widstruct = internal::raw_traits::with_args<T, strict>::any_widstruct;

    template <typename DirT, typename TB = GetWidTypeOrVoid<std::remove_cvref_t<DirT>>>
    CPP_KOSTYL constexpr decltype(auto) unwrap_val(DirT widst) noexcept {
        static_assert(!std::same_as<TB, void>, "Unexpected argument: not widstruct");
        return static_cast<cstd::copy_cv_nref<TB, DirT>>(widst);
    }

    template <typename DirT, typename TB = GetWidTypeOrVoid<std::remove_cvref_t<DirT>>>
    CPP_KOSTYL constexpr decltype(auto) unwrap(DirT&& widst) noexcept {
        static_assert(!std::same_as<TB, void>, "Unexpected argument: not widstruct");
        return static_cast<cstd::copy_cv_eref<TB, DirT&&>>(std::forward<DirT>(widst));
    }

    // ccompl value part
    template <typename T, T value>
    struct ccompl_value {
        IMPLICIT constexpr operator T() const noexcept {
            return value;
        }
    };

    using true_void = ccompl_value<bool, true>;
    using false_void = ccompl_value<bool, false>;

    template <typename T>
    concept bool_or_true = std::same_as<T, bool> || std::same_as<T, true_void>;

    template <typename T, typename Y>
    struct cond_pair : cstd::pair_cast_p_mxn<T, Y> {
        T first;
        Y second;

        constexpr cond_pair(T first, Y second) noexcept : first(first), second(second) {
        }
    };

    template <typename T, typename U, U IU>
    struct cond_pair<T, ccompl_value<U, IU>> : cstd::pair_cast_p_mxn<T, U> {
        T first;
        static constexpr U second = IU;

        constexpr cond_pair(T first, const U sec) noexcept : first(first) {
            assert(sec == second);
        }

        explicit constexpr cond_pair(T first) noexcept : first(first) {
        }
    };
} // namespace gstd
