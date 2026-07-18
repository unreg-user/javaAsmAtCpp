module;

#include "../macro.h"
#include <variant>

export module cstd_variant;

template <bool hasNullptr = false, typename... DerefTypes>
using deref_to_ref_variant = std::conditional_t<
    hasNullptr,
    std::variant<nullptr_t, DerefTypes*...>,
    std::variant<DerefTypes*...>>;

export {
    namespace cstd {
        namespace mixins {
            template <size_t emptyTypeIdx = 0, typename EmptyType>
            struct emptiable_variant_mixin {
                using EmptyT = EmptyType;
                constexpr size_t emptyIdx = emptyTypeIdx;

                NODISCARD constexpr bool is_empty(this auto&& self) noexcept {
                    return self.index() == emptyTypeIdx;
                }

                NODISCARD constexpr bool is_not_empty(this auto&& self) noexcept {
                    return !self.is_empty();
                }
            };

            template <typename DerefEmptyType, size_t emptyTypeIdx = 0>
            struct emptiable_variant_of_ptrs_mixin : emptiable_variant_mixin<emptyTypeIdx, DerefEmptyType*> {
                using DerefEmptyT = DerefEmptyType;
            };

            template <size_t nullTypeIdx = 0>
            struct nullable_variant_of_ptrs_mixin {
                constexpr size_t nullIdx = nullTypeIdx;

                NODISCARD constexpr bool is_null(this auto&& self) noexcept {
                    return self.index() == nullTypeIdx;
                }

                NODISCARD constexpr bool is_not_null(this auto&& self) noexcept {
                    return !self.is_null();
                }
            };

            template <typename DerefEmptyType, size_t nullTypeIdx = 0, size_t emptyTypeIdx = 1>
            struct emptiable_nullable_variant_of_ptrs_mixin :
                    emptiable_variant_of_ptrs_mixin<DerefEmptyType, emptyTypeIdx>,
                    nullable_variant_of_ptrs_mixin<nullTypeIdx> {
                NODISCARD constexpr bool is_emptnull(this auto&& self) noexcept {
                    return self.is_null() || self.is_empty();
                }

                NODISCARD constexpr bool is_not_emptnull(this auto&& self) noexcept {
                    return !self.is_emptnull();
                }
            };
        }


        template <bool hasNullptr = false, typename... DerefTypes>
        struct variant_of_ptrs : deref_to_ref_variant<hasNullptr, DerefTypes> {
            using deref_to_ref_variant<hasNullptr, DerefTypes...>::variant;

            NODISCARD constexpr void* get_raw_ptr() const noexcept {
                return std::visit([] (auto self) {
                    return static_cast<void*>(self);
                }, *this);
            }

            constexpr void destruct_deref() const noexcept {
                std::visit([] (const auto* self) {
                    delete self;
                }, *this);
            }

            template <typename Callable>
            NODISCARD constexpr auto deref_visit(Callable callable) const noexcept {
                return std::visit([=] (auto self) {
                    return callable(*self);
                }, *this);
            }

            template <typename DerefType>
            NODISCARD constexpr auto is_holds_alternative_ref() const noexcept {
                return std::holds_alternative<DerefType*>(*this);
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
    }
}
