module;

#include <ostream>
#include <optional>
#include <variant>
#include "macro.h"

export module cstd_variant;

import simple_cstd;
import dstd;

export namespace cstd {
    template <typename EmptyType, size_t emptyTypeIdx = 0>
    struct emptiable_variant_p_mxn {
        using EmptyT = EmptyType;
        static constexpr size_t emptyIdx = emptyTypeIdx;

        NODISCARD constexpr bool is_empty(this auto&& self) noexcept {
            return self.index() == emptyTypeIdx;
        }

        NODISCARD constexpr bool is_not_empty(this auto&& self) noexcept {
            return !self.is_empty();
        }
    };

    enum class EmptyValueMode : uint8_t { HAS, HAS_IGNORE_VISIT, HASNT };

    namespace _internal {
        template <size_t, EmptyValueMode, typename...>
        struct _0_variant_of_ptrs_and_little {};
    } // namespace _internal

    template <size_t little = ptr_size, EmptyValueMode emptyValueMode = EmptyValueMode::HASNT, typename... DerefTypes>
    struct variant_of_ptrs_and_little
        : std::variant<OptPtrTBy<DerefTypes, little>...>,
          std::conditional_t<emptyValueMode != EmptyValueMode::HASNT, emptiable_variant_p_mxn<DerefTypes...[0]>,
                             _internal::_0_variant_of_ptrs_and_little<little, emptyValueMode, DerefTypes...>> {
        static_assert(sizeof...(DerefTypes) >= 1);

        using varT = std::variant<OptPtrTBy<DerefTypes, little>...>;
        using varT::varT;

        template <typename DerefType>
        using GetOptimType = OptPtrTBy<DerefType, little>;

    private:
        using EmptyType = GetOptimType<DerefTypes...[0]>;

        static constexpr size_t firstFullIdx = (emptyValueMode == EmptyValueMode::HASNT ? 0 : 1);
        static constexpr bool hasFullType = (sizeof...(DerefTypes) > firstFullIdx);

        using UtilsWithIgnore = std::conditional_t<emptyValueMode == EmptyValueMode::HAS_IGNORE_VISIT,
                                                   utils_with_ignore<EmptyType>, utils_with_ignore<>>;

        static_assert(!(emptyValueMode == EmptyValueMode::HAS_IGNORE_VISIT && !hasFullType),
                      "Hasn't full type (first no empty) in empty ignore variant");

    public:
        template <bool empIgnoreIfShould = true, typename DirCallable>
        NODISCARD F_INLINE static auto wrap_visitor_lam(DirCallable&& callable) noexcept {
            using Callable = std::remove_cvref_t<DirCallable>;
            if constexpr (empIgnoreIfShould && emptyValueMode == EmptyValueMode::HAS_IGNORE_VISIT) {
                return typename UtilsWithIgnore::template with_full_type<GetOptimType<DerefTypes...[firstFullIdx]>>::template deref_lam_t<Callable>{std::forward<DirCallable>(callable)};
            } else {
                return typename UtilsWithIgnore::template deref_lam_t<Callable>{std::forward<DirCallable>(callable)};
            }
        }

        constexpr void destruct_deref(this auto&& self) noexcept {
            std::visit(overload{[](auto*& slf) {
                                    delete slf;
                                    slf = nullptr;
                                },
                                [](auto*&& slf) {
                                    delete slf;
                                },
                                [](auto&&) {}},
                       self);
        }

        template <typename DerefType>
        NODISCARD constexpr auto is_holds_alternative() const noexcept {
            return std::holds_alternative<GetOptimType<DerefType>>(*this);
        }

        NODISCARD constexpr auto is_holds_empty() const noexcept {
            return std::holds_alternative<EmptyType>(*this);
        }

        template <typename Callable>
        constexpr decltype(auto) visit(this auto&& self, Callable&& callable) noexcept {
            return std::visit(self.wrap_visitor_lam(std::forward<Callable>(callable)),
                              std::forward_like<decltype(self)>(self));
        }

        template <size_t idx>
        constexpr decltype(auto) get(this auto&& self) noexcept {
            return UtilsWithIgnore::valueDereferLam(std::get<idx>(std::forward_like<decltype(self)>(self)));
        }

        friend std::ostream& operator<<(std::ostream& os, const variant_of_ptrs_and_little& self) {
            os << "vptral<" << self.index() << ">";
            auto lambda0 = overload{[&os](auto* slf) {
                                        os << "*(";
                                        os << (*slf);
                                        os << ")";
                                    },
                                    [&os]<typename T0>(T0&& slf) {
                                        os << "(";
                                        os << std::forward<T0>(slf);
                                        os << ")";
                                    }};
            auto lambda = add_emp_to_visit_lambda(lambda0, [&os](EmptyType) { os << "EMPTY()"; });
            std::visit(lambda, std::forward_like<decltype(self)>(self));
            return os;
        }

        IMPLICIT variant_of_ptrs_and_little(const variant_of_ptrs_and_little& other) noexcept : varT(other) {
        }
        IMPLICIT variant_of_ptrs_and_little(variant_of_ptrs_and_little&& other) noexcept : varT(std::move(other)) {
        }

        variant_of_ptrs_and_little& operator=(const variant_of_ptrs_and_little&) noexcept = default;
        variant_of_ptrs_and_little& operator=(variant_of_ptrs_and_little&&) noexcept = default;

        IMPLICIT variant_of_ptrs_and_little() noexcept
            requires(emptyValueMode != EmptyValueMode::HASNT) &&
                    (!cstd::larger_than<EmptyType, little>) && std::is_default_constructible_v<EmptyType>
            : varT(EmptyType{}) {
        }

        IMPLICIT variant_of_ptrs_and_little() noexcept
            requires(emptyValueMode != EmptyValueMode::HASNT) && cstd::larger_than<EmptyType, little> &&
                    std::is_default_constructible_v<EmptyType>
            : varT(cstd::copy_to_heap(EmptyType{})) {
        }
        template <typename T>
        IMPLICIT constexpr variant_of_ptrs_and_little(const T value) noexcept
            requires(!cstd::larger_than<T, little>) && (!std::is_same_v<T, variant_of_ptrs_and_little>)
            : varT(value) {
        }

        template <typename T>
        // NOLINTNEXTLINE(*-forwarding-reference-overload)
        IMPLICIT variant_of_ptrs_and_little(T&& value) noexcept
            requires cstd::larger_than<std::remove_cvref_t<T>, little> &&
                     (!std::same_as<std::remove_cvref_t<T>, variant_of_ptrs_and_little>)
            : varT(cstd::copy_to_heap(std::forward<T>(value))) {
        }
    };

    // Jaac Wrapper
    template <typename... Types>
    struct jcw_variant : std::variant<Types...> {
        using std::variant<Types...>::variant;

        template <typename Callable>
        NODISCARD decltype(auto) visit(this auto&& self, Callable&& callable) {
            return std::visit(std::forward<Callable>(callable), std::forward_like<decltype(self)>(self));
        }

        template <size_t idx>
        NODISCARD constexpr decltype(auto) get(this auto&& self) noexcept {
            return std::get<idx>(std::forward_like<decltype(self)>(self));
        }
    };

    template <typename... DerefTypes>
    using variant_of_ptrs_and_little_t_ptr = variant_of_ptrs_and_little<ptr_size, EmptyValueMode::HASNT, DerefTypes...>;

    template <typename... DerefTypes>
    using emp_ignore_variant_of_ptrs_and_little_t_ptr =
            variant_of_ptrs_and_little<ptr_size, EmptyValueMode::HAS_IGNORE_VISIT, DerefTypes...>;

    template <typename Variant>
    struct var_opt {
        Variant var;

        using EmptyT = Variant::EmptyT;

        var_opt() noexcept : var(Variant{EmptyT{}}) {
        }

        IMPLICIT var_opt(Variant var) noexcept : var(var) {
        }

        NODISCARD explicit operator bool() const noexcept {
            return !var.is_empty();
        }

        NODISCARD decltype(auto) operator*(this auto&& self) noexcept {
            return std::forward_like<decltype(self)>(self.var);
        }

        IMPLICIT operator std::optional<Variant>() const noexcept {
            return std::optional{var};
        }
    };
} // namespace cstd
