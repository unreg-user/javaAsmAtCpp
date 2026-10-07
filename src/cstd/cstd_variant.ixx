module;

#include <cassert>
#include <iostream>
#include <optional>
#include <variant>
#include "macro.h"

export module cstd_variant;

import simple_cstd;
import dstd;

template <bool hasNullptr = false, typename... DerefTypes>
using deref_to_ref_variant =
        std::conditional_t<hasNullptr, std::variant<nullptr_t, DerefTypes*...>, std::variant<DerefTypes*...>>;

export namespace cstd {
    namespace mixins {
        template <typename EmptyType, size_t emptyTypeIdx = 0>
        struct emptiable_variant_mixin {
            using EmptyT = EmptyType;
            static constexpr size_t emptyIdx = emptyTypeIdx;

            NODISCARD constexpr bool is_empty(this auto&& self) noexcept {
                return self.index() == emptyTypeIdx;
            }

            NODISCARD constexpr bool is_not_empty(this auto&& self) noexcept {
                return !self.is_empty();
            }
        };

        template <typename DerefEmptyType, size_t emptyTypeIdx = 0>
        struct emptiable_variant_of_ptrs_mixin : emptiable_variant_mixin<DerefEmptyType*, emptyTypeIdx> {
            using DerefEmptyT = DerefEmptyType;
        };

        template <size_t nullTypeIdx = 0>
        struct nullable_variant_of_ptrs_mixin {
            static constexpr size_t nullIdx = nullTypeIdx;

            NODISCARD constexpr bool is_null(this auto&& self) noexcept {
                return self.index() == nullTypeIdx;
            }

            NODISCARD constexpr bool is_not_null(this auto&& self) noexcept {
                return !self.is_null();
            }
        };

        template <typename DerefEmptyType, size_t nullTypeIdx = 0, size_t emptyTypeIdx = 1>
        struct emptiable_nullable_variant_of_ptrs_mixin : emptiable_variant_of_ptrs_mixin<DerefEmptyType, emptyTypeIdx>,
                                                          nullable_variant_of_ptrs_mixin<nullTypeIdx> {
            NODISCARD constexpr bool is_emptnull(this auto&& self) noexcept {
                return self.is_null() || self.is_empty();
            }

            NODISCARD constexpr bool is_not_emptnull(this auto&& self) noexcept {
                return !self.is_emptnull();
            }
        };
    } // namespace mixins


    template <bool hasNullptr = false, typename... DerefTypes>
    struct variant_of_ptrs : deref_to_ref_variant<hasNullptr, DerefTypes...> {
        using deref_to_ref_variant<hasNullptr, DerefTypes...>::deref_to_ref_variant;

        NODISCARD constexpr void* get_raw_ptr() const noexcept {
            return std::visit([](auto self) { return static_cast<void*>(self); }, *this);
        }

        constexpr void destruct_deref() const noexcept {
            std::visit(overload{[](const auto* self) { delete self; }, [](const nullptr_t) {}}, *this);
        }

        constexpr auto deref_visit_with_nullptr(auto&& callable, auto&& null_callable) const noexcept {
            return std::visit(
                    overload{[&](auto* self) { return callable(*self); }, [&](nullptr_t) { return null_callable(); }},
                    *this);
        }

        template <typename Callable>
        constexpr auto deref_visit(Callable&& callable) const noexcept {
            using Ret = dstd::call::MakeCallableTraits<Callable>::RetT;
            return deref_visit_with_nullptr(std::forward<Callable>(callable), []() -> Ret {
                assert(false);
                // ReSharper disable once CppFunctionDoesntReturnValue
            });
        }

        template <typename DerefType>
        NODISCARD constexpr auto is_holds_alternative_ref() const noexcept {
            return std::holds_alternative<DerefType*>(*this);
        }

        friend std::ostream& operator<<(std::ostream& os, const variant_of_ptrs& self) {
            os << "vptr<" << self.index() << ">";
            std::visit(overload{[&](auto* selfVal) {
                                    os << "(";
                                    selfVal->operatorKK(os);
                                    os << ")";
                                },
                                [](nullptr_t) {}},
                       self);
            return os;
        }
    };

    template <typename DerefEmptyType, typename... DerefTypes>
    struct variant_of_emptiable_ptrs : variant_of_ptrs<false, DerefEmptyType, DerefTypes...>,
                                       mixins::emptiable_variant_of_ptrs_mixin<DerefEmptyType> {
        using variant_of_ptrs<true, DerefTypes...>::variant_of_ptrs;
    };

    template <typename... DerefTypes>
    struct variant_of_nullable_ptrs : variant_of_ptrs<true, DerefTypes...>, mixins::nullable_variant_of_ptrs_mixin<> {
        using variant_of_ptrs<true, DerefTypes...>::variant_of_ptrs;
    };

    template <typename DerefEmptyType, typename... DerefTypes>
    struct variant_of_nullable_emptiable_ptrs : variant_of_nullable_ptrs<DerefEmptyType, DerefTypes...>,
                                                mixins::emptiable_nullable_variant_of_ptrs_mixin<DerefEmptyType> {
        using variant_of_nullable_ptrs<DerefEmptyType, DerefTypes...>::variant_of_nullable_ptrs;
    };

    enum class EmptyValueMode : uint8_t { HAS, HAS_IGNORE_VISIT, HASNT };

    namespace _internal {
        template <size_t, EmptyValueMode, typename...>
        struct _0variant_of_ptrs_and_little {};
    }

    template <size_t little = ptr_size, EmptyValueMode emptyValueMode = EmptyValueMode::HASNT, typename... DerefTypes>
    struct variant_of_ptrs_and_little : std::variant<OptPtrTBy<DerefTypes, little>...>, std::conditional_t<emptyValueMode != EmptyValueMode::HASNT, mixins::emptiable_variant_mixin<DerefTypes...[0]>, _internal::_0variant_of_ptrs_and_little<little, emptyValueMode, DerefTypes...>> {
        using varT = std::variant<OptPtrTBy<DerefTypes, little>...>;

        template <typename DerefType>
        using GetOptType = OptPtrTBy<DerefType, little>;

    private:
        using EmptyType = GetOptType<DerefTypes...[0]>;
        using AnyFullType = GetOptType<DerefTypes...[1]>;

    public:
        NODISCARD F_INLINE static auto add_emp_to_visit_lambda(auto&& callable, auto&& emp_callable) noexcept {
            if constexpr (emptyValueMode != EmptyValueMode::HASNT) {
                return overload {
                    emp_callable, callable
                };
            } else {
                return callable;
            }
        }

        NODISCARD F_INLINE static auto add_emp_ignore_to_visit_lambda(auto&& callable, auto&& emp_callable) noexcept {
            if constexpr (emptyValueMode == EmptyValueMode::HAS_IGNORE_VISIT) {
                return overload {
                    emp_callable, callable
                };
            } else {
                return callable;
            }
        }

        template <bool mustHandleEmpty, typename Callable>
        NODISCARD F_INLINE static auto emp_ignore_in_visit_lambda(Callable&& callable) noexcept {
            if constexpr (mustHandleEmpty) {
                using RetT = decltype(std::forward<Callable>(callable)(std::declval<AnyFullType>()));
                return add_emp_ignore_to_visit_lambda(std::forward<Callable>(callable), [](EmptyType) -> RetT {
                    std::unreachable();
                });
            } else {
                return []{};
            }
        }

        constexpr void destruct_deref() const noexcept {
            std::visit(overload{[](const auto* self) { delete self; }, [](const auto) {}}, *this);
        }

        template <typename DerefType>
        NODISCARD constexpr auto is_holds_alternative() const noexcept {
            return std::holds_alternative<GetOptType<DerefType>>(*this);
        }

        template <bool mustHandleEmpty = false, typename Callable>
        constexpr auto deref_visit(Callable&& callable) const noexcept {
            return std::visit(this->template emp_ignore_in_visit_lambda<mustHandleEmpty>(
                    overload{[&]<typename T>(T self)
                                 requires std::is_pointer_v<T>
                             { return std::forward<Callable>(callable)(*self); },
                             [&](const auto self) { return std::forward<Callable>(callable)(self); }}),
                    *this);
        }

        template <bool mustHandleEmpty = false, typename BigC, typename SmallC>
        constexpr auto sep_visit(BigC&& bigC, BigC&& smallC) const noexcept {
            return std::visit(this->template emp_ignore_in_visit_lambda<mustHandleEmpty>(overload{
                                      [&bigC](const auto* self) { return std::forward<BigC>(bigC)(self); },
                                      [&smallC](const auto self) { return std::forward<SmallC>(smallC)(self); }}),
                              *this);
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
            auto lambda = add_emp_to_visit_lambda(lambda0, [&os](EmptyType) {
                os << "EMPTY()";
            });
            std::visit(lambda, self);
            return os;
        }

        /*implicit*/ variant_of_ptrs_and_little(const variant_of_ptrs_and_little& other) noexcept
            : varT(other) {}
        /*implicit*/ variant_of_ptrs_and_little(variant_of_ptrs_and_little&& other) noexcept
            : varT(std::move(other)) {}

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
            requires (!cstd::larger_than<T, little>) && (!std::is_same_v<T, variant_of_ptrs_and_little>)
            : varT(value) {
        }

        template <typename T>
        //NOLINTNEXTLINE(*-forwarding-reference-overload)
        /*implicit*/ variant_of_ptrs_and_little(T&& value) noexcept
            requires cstd::larger_than<std::remove_cvref_t<T>, little> && (!std::same_as<std::remove_cvref_t<T>, variant_of_ptrs_and_little>)
            : varT(cstd::copy_to_heap(std::forward<T>(value))) {
        }

        /*implicit*/ variant_of_ptrs_and_little() noexcept requires (emptyValueMode != EmptyValueMode::HASNT) && (!cstd::larger_than<EmptyType, little>) && std::is_default_constructible_v<EmptyType>
            : varT(EmptyType{}) {}

        /*implicit*/ variant_of_ptrs_and_little() noexcept requires (emptyValueMode != EmptyValueMode::HASNT) && cstd::larger_than<EmptyType, little> && std::is_default_constructible_v<EmptyType>
            : varT(cstd::copy_to_heap(EmptyType{})) {}
    };

    template <typename... DerefTypes>
    using variant_of_ptrs_and_little_t_ptr = variant_of_ptrs_and_little<ptr_size, EmptyValueMode::HASNT, DerefTypes...>;

    template <typename... DerefTypes>
    using emp_ignore_variant_of_ptrs_and_little_t_ptr = variant_of_ptrs_and_little<ptr_size, EmptyValueMode::HAS_IGNORE_VISIT, DerefTypes...>;

    template <typename Var>
    struct var_opt {
        Var var;

        using EmptyT = Var::EmptyT;

        var_opt() noexcept : var(Var{EmptyT{}}) {
        }

        /*implicit*/ var_opt(Var var) noexcept : var(var) {
        }

        NODISCARD explicit operator bool() const noexcept {
            return !var.is_empty();
        }

        NODISCARD auto operator*() const noexcept {
            return var;
        }
    };
} // namespace cstd
