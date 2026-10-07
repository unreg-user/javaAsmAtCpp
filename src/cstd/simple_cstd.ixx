module;

#include <array>
#include <bit>
#include <cstdint>
#include <functional>
#include <span>
#include <type_traits>
#include <variant>
#include <xtr1common>

#include "macro.h"

export module simple_cstd;

export struct Void {};

export namespace cstd {
    template <typename... Callables>
    struct overload : Callables... {
        using Callables::operator()...;
    };

    template <typename T1, typename T2>
    using CopyRef = std::conditional_t<std::is_lvalue_reference_v<T1>, std::remove_reference_t<T2>&,
                                       std::remove_reference_t<T2>&&>;

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

    template <typename T2>
    struct cast_lambda_t {
        template <typename T1>
        NODISCARD decltype(auto) operator()(T1&& value) const noexcept
            requires larger_t_ptr<std::remove_cvref<T1>>
        {
            return static_cast<CopyRef<T1, T2>>(value);
        }

        template <typename T1>
        NODISCARD F_INLINE T2 operator()(T1 value) const noexcept
            requires(!larger_t_ptr<std::remove_cvref<T1>>)
        {
            return static_cast<T2>(value);
        }
    };

    template <typename T2>
    struct ext_cast_lambda_t {
        template <typename T1>
        NODISCARD F_INLINE T2 operator()(T1&& value) const noexcept
            requires larger_t_ptr<std::remove_cvref<T1>>
        {
            return T2{value};
        }

        template <typename T1>
        NODISCARD F_INLINE T2 operator()(T1 value) const noexcept
            requires(!larger_t_ptr<std::remove_cvref<T1>>)
        {
            return T2{value};
        }
    };

    struct identify_lambda_t {
        template <typename T1>
        NODISCARD F_INLINE decltype(auto) operator()(T1&& value) const noexcept
            requires larger_t_ptr<std::remove_cvref<T1>>
        {
            return value;
        }

        template <typename T1>
        NODISCARD F_INLINE T1 operator()(T1 value) const noexcept
            requires(!larger_t_ptr<std::remove_cvref<T1>>)
        {
            return value;
        }
    };

    constexpr auto same_as_l2args = []<typename T, typename Y>() constexpr { return std::same_as<T, Y>; };
    template <typename T>
    constexpr auto same_as_l1arg = []<typename Y>() constexpr { return std::same_as<T, Y>; };

    constexpr auto destruct1_lambda = [](auto value) {
        auto [v] = value;
        return v;
    };

    template <typename T>
    using destruct1_type = decltype(destruct1_lambda(std::declval<T>()));

    template <typename T>
    constexpr bool is_func_ptr = (std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>) ||
                                 std::is_member_function_pointer_v<T>;

    template <typename T2>
    constexpr auto cast_lambda = cast_lambda_t<T2>{};
    template <typename T2>
    constexpr auto ext_cast_lambda = ext_cast_lambda_t<T2>{};
    constexpr auto identify_lambda = identify_lambda_t{};
    constexpr auto byteswap_v_func = []<typename T>(T value) -> T {
        if constexpr (requires { std::byteswap(value); }) {
            return std::byteswap(value);
        } else {
            auto [value_field] = value;
            return T{std::byteswap(value_field)};
        }
    };

    template <typename Callable>
    struct _func_or_false_line_Handler {
        Callable callable;

        template <typename Callable2>
        auto operator|(_func_or_false_line_Handler<Callable2> call2Cont) {
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
                    return _func_or_false_line_Handler<decltype(l)>{l};
                }
            }
        }
    };

    template <typename... Callables>
    constexpr auto func_or_false_line(Callables... callables) {
        if constexpr (sizeof...(Callables) > 0) {
            return (... | _func_or_false_line_Handler{callables}).callable;
        } else {
            return false;
        }
    }

    template <typename... Callables>
    constexpr void call(Callables&&... callables) {
        (..., callables());
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
        if (sizeof(T1) % sizeof(T2) != 0)
            static_assert("cannot convert");
        constexpr auto size = sizeof(T1) / sizeof(T2);
        using RetType = std::span<T2, size>;

        auto ptr = std::bit_cast<T2*>(&value);
        return RetType{ptr, size};
    }

    template <typename T2, typename T1>
    constexpr auto convert_to_array(T1&& value) {
        if (sizeof(T1) % sizeof(T2) != 0)
            static_assert("cannot convert");
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

    template <typename TD>
    NODISCARD auto copy_to_heap_if_noempty(TD&& obj) noexcept {
        using T = std::remove_cvref_t<TD>;
        if constexpr (std::is_empty_v<T>) {
            return new T(std::forward<TD>(obj));
        } else {
            return static_cast<T*>(nullptr);
        }
    }

    template <typename T, size_t maxSize>
    struct vec_array {
        std::array<T, maxSize> array{};
        size_t size = 0;

        using iterator = std::_Array_iterator<T, maxSize>;
        using const_iterator = std::_Array_const_iterator<T, maxSize>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr void push_back(const T& elem) {
            array[size++] = elem;
        }

        constexpr void push_back(T&& elem) {
            array[size++] = std::move(elem);
        }

        constexpr decltype(auto) operator[](this auto&& self, size_t i) {
            return self.array[i];
        }

        NODISCARD constexpr iterator end() noexcept {
            return iterator(array._Elems, size);
        }

        NODISCARD constexpr const_iterator end() const noexcept {
            return const_iterator(array._Elems, size);
        }

        NODISCARD constexpr const_iterator cend() const noexcept {
            return end();
        }

        NODISCARD constexpr auto begin(this auto&& self) {
            return self.array.begin();
        }

        NODISCARD constexpr auto cbegin(this auto&& self) {
            return self.array.cbegin();
        }
    };

    template <typename T>
    decltype(auto) forward_cpy(T value) {
        if constexpr (std::is_rvalue_reference_v<T>) {
            return static_cast<T&&>(value);
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return static_cast<T&>(value);
        } else {
            return value;
        }
    }

    void add_reserve(auto& vec, size_t size) {
        vec.reserve(vec.size() + size);
    }
} // namespace cstd
