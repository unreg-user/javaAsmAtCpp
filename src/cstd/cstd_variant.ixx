module;

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <variant>
#include "macro.h"

export module cstd_variant;

import simple_cstd;
import dstd;

template <bool hasNullptr = false, typename... DerefTypes>
using deref_to_ref_variant =
        std::conditional_t<hasNullptr, std::variant<nullptr_t, DerefTypes*...>, std::variant<DerefTypes*...>>;

export {
    namespace cstd {
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
            struct emptiable_nullable_variant_of_ptrs_mixin
                : emptiable_variant_of_ptrs_mixin<DerefEmptyType, emptyTypeIdx>,
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
                return std::visit(overload{[&](auto* self) { return callable(*self); },
                                           [&](nullptr_t) { return null_callable(); }},
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
        struct variant_of_nullable_ptrs : variant_of_ptrs<true, DerefTypes...>,
                                          mixins::nullable_variant_of_ptrs_mixin<> {
            using variant_of_ptrs<true, DerefTypes...>::variant_of_ptrs;
        };

        template <typename DerefEmptyType, typename... DerefTypes>
        struct variant_of_nullable_emptiable_ptrs : variant_of_nullable_ptrs<DerefEmptyType, DerefTypes...>,
                                                    mixins::emptiable_nullable_variant_of_ptrs_mixin<DerefEmptyType> {
            using variant_of_nullable_ptrs<DerefEmptyType, DerefTypes...>::variant_of_nullable_ptrs;
        };

        template <typename... DerefTypes, size_t little = ptr_size>
        struct variant_of_ptrs_and_little : std::variant<OptPtrTBy<DerefTypes, little>...> {
            using varT = std::variant<OptPtrTBy<DerefTypes, little>...>;

            template <typename DerefType>
            using GetOptType = OptPtrTBy<DerefType, little>;

            template <typename T>
            explicit variant_of_ptrs_and_little(T value) noexcept requires (!cstd::larger_than<T, little>) : varT(value) {}

            template <typename T>
            explicit variant_of_ptrs_and_little(T&& value) noexcept requires cstd::larger_than<T, little> : varT(cstd::copy_to_heap(value)) {}

            constexpr void destruct_deref() const noexcept {
                std::visit(overload{[](const auto* self) { delete self; }, [](const auto) {}}, *this);
            }

            template <typename DerefType>
            NODISCARD constexpr auto is_holds_alternative() const noexcept {
                return std::holds_alternative<GetOptType<DerefType>>(*this);
            }

            template <typename Callable>
            constexpr auto deref_visit(Callable&& callable) const noexcept {
                std::visit(overload{[&](const auto* self) {
                    return std::forward<Callable>(callable)(*self);
                }, [&](const auto self) {
                    return std::forward<Callable>(callable)(self);
                }}, *this);
            }

            template <typename BigC, typename SmallC>
            constexpr auto sep_visit(BigC&& bigC, BigC&& smallC) const noexcept {
                std::visit(overload{[&bigC](const auto* self) {
                    return std::forward<BigC>(bigC)(self);
                }, [&smallC](const auto self) {
                    return std::forward<SmallC>(smallC)(self);
                }}, *this);
            }

            friend std::ostream& operator<<(std::ostream& os, const variant_of_ptrs_and_little self) {
                os << "vptral<" << self.index() << ">";
                std::visit(overload{[&](auto* selfVal) {
                                        os << "*(";
                                        selfVal->operatorKK(os);
                                        os << ")";
                                    },
                                    [&](auto selfVal) {
                                        os << "(";
                                        selfVal.operatorKK(os);
                                        os << ")";
                                    }},
                           self);
                return os;
            }
        };
    } // namespace cstd
}
