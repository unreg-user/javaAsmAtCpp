module;

#include <array>
#include <bit>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <type_traits>

#include "macro.h"
import macro_u;

export module simple_cstd;

export {
    using f32_t = float;
    using f64_t = double;
}

export namespace cstd {
    template <typename... Callables>
    struct overload : Callables... {
        using Callables::operator()...;
    };

    template <typename...>
    concept dependent_false = false;

    CPP_KOSTYL template <typename To, typename From>
    using apply_exists_ref = std::conditional_t<std::is_lvalue_reference_v<From>, To&, To&&>;

    CPP_KOSTYL template <typename To, typename From>
    using apply_ref = std::conditional_t<std::is_lvalue_reference_v<From>, To&,
                                         std::conditional_t<std::is_rvalue_reference_v<From>, To&&, To>>;

    CPP_KOSTYL template <typename To, typename From>
    using apply_const_nref = std::conditional_t<std::is_const_v<From>, const To, To>;

    CPP_KOSTYL template <typename To, typename From>
    using apply_const = apply_const_nref<To, std::remove_reference_t<From>>;

#if USE_VOLATILE
    CPP_KOSTYL template <typename To, typename From>
    using apply_volatile_nref = std::conditional_t<std::is_volatile_v<From>, volatile To, To>;

    CPP_KOSTYL template <typename To, typename From>
    using apply_volatile = apply_volatile_nref<To, std::remove_reference_t<From>>;
#else
    CPP_KOSTYL template <typename To, typename From>
    using apply_volatile_nref = To;

    CPP_KOSTYL template <typename To, typename From>
    using apply_volatile = To;
#endif

    CPP_KOSTYL template <typename To, typename From>
    using apply_cvref = apply_ref<apply_const<apply_volatile<To, From>, From>, From>;

    CPP_KOSTYL template <typename To, typename From>
    using apply_cv_eref = apply_exists_ref<apply_const<apply_volatile<To, From>, From>, From>;

    CPP_KOSTYL template <typename To, typename From>
    using apply_cv_nref = apply_const_nref<apply_volatile_nref<To, From>, From>;

    CPP_KOSTYL template <typename To, typename From>
    using copy_cvref = apply_cvref<std::remove_cvref_t<To>, From>;

    CPP_KOSTYL template <typename To, typename From>
    using copy_cv_eref = apply_cv_eref<std::remove_cvref_t<To>, From>;

    CPP_KOSTYL template <typename To, typename From>
    using copy_cv_nref = apply_cv_nref<std::remove_cv_t<To>, From>;

    template <typename T, size_t than>
    concept larger_than = sizeof(T) > than;

    template <typename T, typename T2>
    concept larger_t_type = larger_than<T, sizeof(T2)>;

    template <typename T>
    concept larger_t_ptr = larger_t_type<T, void*>;

    constexpr size_t ptr_size = sizeof(void*);

    template <typename T, size_t little>
    using OptPtrTBy = std::conditional_t<larger_than<T, little>, T*, T>;

    template <typename T>
    using OptPtrT = std::conditional_t<larger_t_ptr<T>, T*, T>;

    template <typename T>
    using GetMinNumStrUType = std::conditional_t<std::same_as<T, uint8_t>, uint16_t, T>;

    template <typename T, typename Y>
    struct pair_cast_p_mxn {
        constexpr std::pair<T, Y> to_pair(this auto&& self) {
            return std::pair<T, Y>{self.first, self.second};
        }

        IMPLICIT constexpr operator std::pair<T, Y>(this auto&& self) noexcept {
            return self.to_pair();
        }
    };

    template <typename T>
    concept is_func_ptr = (std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>) ||
                          std::is_member_function_pointer_v<T>;

    constexpr struct {
        template <typename T1>
        NODISCARD F_INLINE constexpr decltype(auto) operator()(T1&& value) const noexcept
            requires larger_t_ptr<std::remove_cvref<T1>>
        {
            return value;
        }

        template <typename T1>
        NODISCARD F_INLINE constexpr T1 operator()(T1 value) const noexcept
            requires(!larger_t_ptr<std::remove_cvref<T1>>)
        {
            return value;
        }
    } identifyLam{};

    constexpr auto byteswapLam = []<typename T>(T value) -> T {
        if constexpr (requires { std::byteswap(value); }) {
            return std::byteswap(value);
        } else if constexpr (std::is_enum_v<T>) {
            return static_cast<T>(std::byteswap(static_cast<std::underlying_type_t<T>>(value)));
        } /* else {
            CPP_KOSTYL // where is used? I don't remember
            auto [value_field] = value;
            return T{std::byteswap(value_field)};
        } */
        else {
            static_assert(false);
            return {};
        }
    };

    template <typename Callable>
    struct FuncOrFalseLineHandler {
        Callable callable;

        template <typename Callable2>
        auto constexpr operator|(FuncOrFalseLineHandler<Callable2> call2Cont) {
            if constexpr (std::same_as<Callable, bool>) {
                return call2Cont;
            } else {
                if constexpr (std::same_as<Callable2, bool>) {
                    return *this;
                } else {
                    auto l = [=, *this]() mutable {
                        callable();
                        call2Cont.callable();
                    };
                    return FuncOrFalseLineHandler<decltype(l)>{l};
                }
            }
        }
    };

    template <typename... Callables>
    constexpr auto func_or_false_line(Callables... callables) {
        if constexpr (sizeof...(Callables) > 0) {
            return (... | FuncOrFalseLineHandler{callables}).callable;
        } else {
            return false;
        }
    }

    template <typename Callable>
    struct NoFalseCaller {
        Callable& callable;

        void constexpr operator*() const {
            if constexpr (!std::same_as<std::remove_cvref_t<Callable>, bool>) {
                callable();
            }
        }
    };

    template <typename Callable>
    constexpr void call_if_no_false(Callable&& callable) {
        *NoFalseCaller{callable};
    }

    template <bool Cond, typename T>
    constexpr T or_else_falseCPY(T obj) {
        if constexpr (Cond)
            return obj;
        else
            return false;
    }

    template <typename T2, typename T1>
    constexpr auto convert_to_span(T1& value) {
        static_assert(sizeof(T1) % sizeof(T2) == 0);

        constexpr auto size = sizeof(T1) / sizeof(T2);
        using RetType = std::span<T2, size>;

        auto ptr = std::bit_cast<T2*>(&value);
        return RetType{ptr, size};
    }

    template <typename T2, typename T1>
    constexpr auto convert_to_array(T1&& value) {
        static_assert(sizeof(T1) % sizeof(T2) == 0);

        constexpr auto size = sizeof(T1) / sizeof(T2);
        using RetType = std::array<T2, size>;

        return std::bit_cast<RetType>(value);
    }


    // clang-format off
    template <typename T>
    using ConcUPrimT = std::conditional_t<
        std::same_as<T, uint8_t>,
        uint16_t,
        std::conditional_t<
            std::same_as<T, uint16_t>,
            uint32_t,
                std::conditional_t<
                    std::same_as<T, uint32_t>,
                    uint64_t,
                    void
                >
            >
        >;
    // clang-format on

    template <typename T>
    constexpr auto conc_u_prim(T a, T b) noexcept {
        using Ret = ConcUPrimT<T>;
        static_assert(!std::same_as<Ret, void>, "Cannot conc");

        return static_cast<Ret>(static_cast<Ret>(a) << (sizeof(T) * 8) | b);
    }

    template <typename TD>
    NODISCARD auto copy_to_heap(TD&& obj) noexcept {
        using T = std::remove_cvref_t<TD>;
        return new T(std::forward<TD>(obj));
    }

    template <typename ClearT2, typename DirT1>
    NODISCARD F_INLINE decltype(auto) forward_static_cast(DirT1&& t1) noexcept(noexcept(static_cast<copy_cv_eref<ClearT2, DirT1&&>>(std::forward<DirT1>(t1)))) {
        return static_cast<copy_cv_eref<ClearT2, DirT1&&>>(std::forward<DirT1>(t1));
    }

    template <typename TD>
    NODISCARD auto copy_to_heap_if_noempty(TD&& obj) noexcept {
        using T = std::remove_cvref_t<TD>;
        if constexpr (std::is_empty_v<T>) {
            return new T(std::forward<TD>(obj));
        } else {
            return static_cast<T*>(nullptr);
        }
    }

    constexpr void add_reserve(auto& vec, size_t size) {
        vec.reserve(vec.size() + size);
    }

    template <typename Base, typename... PMixins>
    struct ApplyP : Base, PMixins... {
        using Base::Base;
    };

    template <typename Base>
    struct NoneWMxn : Base {
        using Base::Base;

        IMPLICIT NoneWMxn(Base& base) : Base(base) {
        }
        IMPLICIT NoneWMxn(Base&& base) : Base(base) {
        }

        IMPLICIT operator Base&() & {
            return static_cast<Base&>(*this);
        }

        IMPLICIT operator Base&&() && {
            return static_cast<Base&&>(*this);
        }

        IMPLICIT operator const Base&() const& {
            return static_cast<const Base&>(*this);
        }

        IMPLICIT operator const Base&&() const&& {
            return static_cast<const Base&&>(*this);
        }
    };

    struct EmptyStruct {};

    template <bool /*Cond*/, typename /*T*/>
    struct OrEmpty {};

    template <typename T>
    struct OrEmpty<true, T> : T {};

    template <typename T>
    NODISCARD std::optional<T> opt_impl_construct(auto&& from) noexcept {
        ul_assert_nullopt(from);
        return std::optional{T(std::forward_like<decltype(from)>(*from))};
    }

    template <typename... IgnoredTs>
    struct utils_with_ignore {
        template <typename Callable>
        struct deref_lam_t {
        protected:
            Callable callable;

        public:
            template <typename Func>
                requires(!std::same_as<std::remove_cvref_t<Func>, deref_lam_t>) &&
                        std::constructible_from<Callable, Func>
            IMPLICIT constexpr deref_lam_t(Func&& func) noexcept(std::is_nothrow_constructible_v<Callable, Func>) :
                callable(std::forward<Func>(func)) {
            }

            template <typename Ptr>
                requires std::is_pointer_v<std::remove_cvref_t<Ptr>> &&
                         (... && (!std::same_as<IgnoredTs,
                                                std::remove_cv_t<std::remove_pointer_t<std::remove_cvref_t<Ptr>>>>) )
            constexpr decltype(auto)
            operator()(this auto&& self,
                       Ptr&& slf) noexcept(noexcept(std::forward_like<decltype(self)>(self.callable)(*slf))) {
                return std::forward_like<decltype(self)>(self.callable)(*slf);
            }

            template <typename T>
                requires(!std::is_pointer_v<std::remove_cvref_t<T>>) &&
                        (... && (!std::same_as<IgnoredTs, std::remove_cvref_t<T>>) )
            constexpr decltype(auto) operator()(this auto&& self, T&& slf) noexcept(
                    noexcept(std::forward_like<decltype(self)>(self.callable)(std::forward<T>(slf)))) {
                return std::forward_like<decltype(self)>(self.callable)(std::forward<T>(slf));
            }
        };

        template <typename Func>
        deref_lam_t(Func&&) -> deref_lam_t<std::remove_cvref_t<Func>>;

        template <typename FullType>
        struct with_full_type {
            template <typename Callable>
            struct deref_lam_t : utils_with_ignore::deref_lam_t<Callable> {
            private:
                using Base = utils_with_ignore::deref_lam_t<Callable>;

            public:
                using Base::Base;
                using Base::operator();

                template <typename Ptr>
                    requires std::is_pointer_v<std::remove_cvref_t<Ptr>> &&
                             (... || (std::same_as<IgnoredTs,
                                                   std::remove_cv_t<std::remove_pointer_t<std::remove_cvref_t<Ptr>>>>) )
                constexpr auto operator()(UNUSED this auto&& self, Ptr&&) noexcept
                        -> decltype(std::forward_like<decltype(self)>(self.callable)(std::declval<FullType>())) {
                    std::unreachable();
                }

                template <typename T>
                    requires(!std::is_pointer_v<std::remove_cvref_t<T>>) &&
                            (... || (std::same_as<IgnoredTs, std::remove_cvref_t<T>>) )
                constexpr auto operator()(UNUSED this auto&& self, T&&) noexcept
                        -> decltype(std::forward_like<decltype(self)>(self.callable)(std::declval<FullType>())) {
                    std::unreachable();
                }
            };

            template <typename Func>
            deref_lam_t(Func&&) -> deref_lam_t<std::remove_cvref_t<Func>>;

            constexpr static auto valueDereferLam = deref_lam_t{
                    []<typename Value>(Value&& value) -> decltype(auto) { return std::forward<Value>(value); }};
        };

        constexpr static auto valueDereferLam =
                deref_lam_t{[]<typename Value>(Value&& value) -> decltype(auto) { return std::forward<Value>(value); }};
    };

    constexpr struct NotDirty {
    } notDirty{};

    constexpr struct Deleted {
    } deleted{};
} // namespace cstd
