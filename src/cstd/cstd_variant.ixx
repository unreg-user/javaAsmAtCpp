module;

#include <cassert>
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

    enum class EmptyValueMode : uint8_t {
        HAS,
        HAS_IGNORE_VISIT,
        HASNT
    };

    namespace _internal {
        template <size_t, EmptyValueMode, typename...>
        struct _0_variant_of_ptrs_and_little {};
    } // namespace _internal

    template <size_t little = ptr_size, EmptyValueMode emptyValueMode = EmptyValueMode::HASNT, typename... DerefTypes>
    struct variant_of_ptrs_and_little
        : std::variant<OptPtrTBy<DerefTypes, little>...>,
          std::conditional_t<emptyValueMode != EmptyValueMode::HASNT, emptiable_variant_p_mxn<DerefTypes...[0]>,
                             _internal::_0_variant_of_ptrs_and_little<little, emptyValueMode, DerefTypes...>> {
        using varT = std::variant<OptPtrTBy<DerefTypes, little>...>;

        template <typename DerefType>
        using GetOptimType = OptPtrTBy<DerefType, little>;

    private:
        using EmptyType = GetOptimType<DerefTypes...[0]>;
        using AnyFullType = decltype([] {
            if constexpr (sizeof...(DerefTypes) > 1) {
                return dstd::args::TypeWrapper<GetOptimType<DerefTypes...[1]>>{};
            } else {
                return dstd::args::TypeWrapper<void>{};
            }
        }())::InstanceT;

    public:
        NODISCARD F_INLINE static auto add_emp_to_visit_lambda(auto&& callable, auto&& emp_callable) noexcept {
            if constexpr (emptyValueMode != EmptyValueMode::HASNT) {
                return overload{emp_callable, callable};
            } else {
                return callable;
            }
        }

        NODISCARD F_INLINE static auto add_emp_ignore_to_visit_lambda(auto&& callable, auto&& emp_callable) noexcept {
            if constexpr (emptyValueMode == EmptyValueMode::HAS_IGNORE_VISIT) {
                return overload{emp_callable, callable};
            } else {
                return callable;
            }
        }

        template <typename Callable>
            requires(emptyValueMode != EmptyValueMode::HAS_IGNORE_VISIT || !std::same_as<AnyFullType, void>)
        NODISCARD F_INLINE static auto emp_ignore_in_visit_lambda(Callable&& callable) noexcept {
            if constexpr (emptyValueMode == EmptyValueMode::HAS_IGNORE_VISIT) {
                using RetT = decltype(std::forward<Callable>(callable)(std::declval<AnyFullType>()));
                return overload{std::forward<Callable>(callable), [](EmptyType) -> RetT { std::unreachable(); }};
            } else {
                return callable;
            }
        }

        constexpr void destruct_deref() noexcept {
            std::visit(overload{[](auto*& self) {
                                    delete self;
                                    self = nullptr;
                                },
                                [](auto&&) {}},
                       *this);
        }

        template <typename DerefType>
        NODISCARD constexpr auto is_holds_alternative() const noexcept {
            return std::holds_alternative<GetOptimType<DerefType>>(*this);
        }

        NODISCARD constexpr auto is_holds_empty() const noexcept {
            return std::holds_alternative<EmptyType>(*this);
        }

        template <typename Callable>
        constexpr auto deref_visit(this auto&& slf, Callable&& callable) noexcept {
            return std::visit(slf.emp_ignore_in_visit_lambda(overload{
                                      [&]<typename T>(T* self) { return std::forward<Callable>(callable)(*self); },
                                      [&](auto&& self) { return std::forward<Callable>(callable)(self); }}),
                              slf);
        }

        friend std::ostream& operator<<(std::ostream& os, const variant_of_ptrs_and_little self) {
            os << "vptral<" << self.index() << ">";
            auto lambda0 = overload{[&os](auto* selfVal) {
                                        os << "*(";
                                        selfVal->operatorKK(os);
                                        os << ")";
                                    },
                                    [&os](auto selfVal) {
                                        os << "(";
                                        selfVal.operatorKK(os);
                                        os << ")";
                                    }};
            auto lambda = add_emp_to_visit_lambda(lambda0, [&os](EmptyType) { os << "EMPTY()"; });
            std::visit(lambda, self);
            return os;
        }

        /*implicit*/ variant_of_ptrs_and_little(const variant_of_ptrs_and_little& other) noexcept : varT(other) {
        }
        /*implicit*/ variant_of_ptrs_and_little(variant_of_ptrs_and_little&& other) noexcept : varT(std::move(other)) {
        }

        variant_of_ptrs_and_little& operator=(const variant_of_ptrs_and_little& other) noexcept {
            if (this != &other) {
                varT::operator=(other);
            }
            return *this;
        }

        variant_of_ptrs_and_little& operator=(variant_of_ptrs_and_little&& other) noexcept {
            if (this != &other) {
                varT::operator=(other);
            }
            return *this;
        }

        template <typename T>
        /*implicit*/ constexpr variant_of_ptrs_and_little(const T value) noexcept
            requires(!cstd::larger_than<T, little>) && (!std::is_same_v<T, variant_of_ptrs_and_little>)
            : varT(value) {
        }

        template <typename T>
        // NOLINTNEXTLINE(*-forwarding-reference-overload)
        /*implicit*/ variant_of_ptrs_and_little(T&& value) noexcept
            requires cstd::larger_than<std::remove_cvref_t<T>, little> &&
                     (!std::same_as<std::remove_cvref_t<T>, variant_of_ptrs_and_little>)
            : varT(cstd::copy_to_heap(std::forward<T>(value))) {
        }

        /*implicit*/ variant_of_ptrs_and_little() noexcept
            requires(emptyValueMode != EmptyValueMode::HASNT) &&
                    (!cstd::larger_than<EmptyType, little>) && std::is_default_constructible_v<EmptyType>
            : varT(EmptyType{}) {
        }

        /*implicit*/ variant_of_ptrs_and_little() noexcept
            requires(emptyValueMode != EmptyValueMode::HASNT) && cstd::larger_than<EmptyType, little> &&
                    std::is_default_constructible_v<EmptyType>
            : varT(cstd::copy_to_heap(EmptyType{})) {
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

        /*implicit*/ var_opt(Variant var) noexcept : var(var) {
        }

        NODISCARD explicit operator bool() const noexcept {
            return !var.is_empty();
        }

        NODISCARD decltype(auto) operator*(this auto&& self) noexcept {
            return self.var;
        }

        /*implicit*/ operator std::optional<Variant>() const noexcept {
            return std::optional{var};
        }
    };
} // namespace cstd
