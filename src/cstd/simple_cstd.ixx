module;

#include <bit>
#include <cstdint>
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
    using CopyRef = std::conditional_t<
        std::is_lvalue_reference_v<T1>,
        std::remove_reference_t<T2>&,
        std::remove_reference_t<T2>&&
            >;

    template <typename T, size_t than>
    concept larger_than = sizeof(T) > than;

    template <typename T, typename T2>
    concept larger_t_type = larger_than<T, sizeof(T2)>;

    template <typename T>
    concept larger_t_ptr = larger_t_type<T, void*>

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
        NODISCARD decltype(auto) operator()(T1&& value) const noexcept requires larger_t_ptr<std::remove_cvref<T1>> {
            return static_cast<CopyRef<T1, T2>>(value);
        }

        template <typename T1>
        NODISCARD F_INLINE T2 operator()(T1 value) const noexcept requires (!larger_t_ptr<std::remove_cvref<T1>>) {
            return static_cast<T2>(value);
        }
    };

    template <typename T2>
    struct ext_cast_lambda_t {
        template <typename T1>
        NODISCARD F_INLINE T2 operator()(T1&& value) const noexcept requires larger_t_ptr<std::remove_cvref<T1>> {
            return T2{value};
        }

        template <typename T1>
        NODISCARD F_INLINE T2 operator()(T1 value) const noexcept requires (!larger_t_ptr<std::remove_cvref<T1>>) {
            return T2{value};
        }
    };

    struct identify_lambda_t {
        template <typename T1>
        NODISCARD F_INLINE decltype(auto) operator()(T1&& value) const noexcept requires larger_t_ptr<std::remove_cvref<T1>> {
            return value;
        }

        template <typename T1>
        NODISCARD F_INLINE T1 operator()(T1 value) const noexcept requires (!larger_t_ptr<std::remove_cvref<T1>>) {
            return value;
        }
    };

    template <typename T2>
    constexpr auto cast_lambda = cast_lambda_t<T2>{};
    template <typename T2>
    constexpr auto ext_cast_lambda = ext_cast_lambda_t<T2>{};
    constexpr auto identify_lambda = identify_lambda_t{};
    constexpr auto byteswap_r_func = [](auto value){ return std::byteswap(value); };

    template <typename T2, typename T1>
    constexpr auto convert_to_span(T1& value) {
        if (sizeof(T1) % sizeof(T2) != 0) static_assert("cannot convert");
        constexpr auto size = sizeof(T1) / sizeof(T2);
        using RetType = std::span<T2, size>;

        auto ptr = std::bit_cast<T2*>(&value);
        return RetType{ptr, size};
    }

    template <typename T2, typename T1>
    constexpr auto convert_to_array(T1&& value) {
        if (sizeof(T1) % sizeof(T2) != 0) static_assert("cannot convert");
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
} // namespace cstd
