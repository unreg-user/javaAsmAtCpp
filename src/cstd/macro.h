#pragma once

// clang-format off

// I hate ISO
#define CPP_KOSTYL
#include <assert.h>

CPP_KOSTYL // that it is not kw
#define COM static constexpr
CPP_KOSTYL
#define COM_BOOL COM bool

#define MACRO_BOOL_CAST(...)  !!(__VA_ARGS__)
#define MACRO_NEGATIVE_BOOL_CAST(...)  !(__VA_ARGS__)

CPP_KOSTYL // this is not separate operator
#define DISCARD (void)

CPP_KOSTYL // this is not separate code block type
#define DW0(...) do { __VA_ARGS__ } while (0)

#define NODISCARD [[nodiscard]]
#define UNUSED [[maybe_unused]]
#define DEPRECATED(text) [[deprecated(text)]]
#define K1 template <typename> typename
#define K2 template <typename, typename> typename
#define KN template <typename...> typename
#define PUBLIC_V_GENERIC_R_AT(path, name) static constexpr auto name = path.name;

#define DEL_CPY(tName) \
        constexpr tName(const tName&) noexcept = delete; \
        constexpr tName& operator=(const tName&) noexcept = delete;

CPP_KOSTYL // that it is not kw
#define IMPLICIT explicit(false)

#if defined(__clang__) || defined(__GNUC__)
    #define F_INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
    #define F_INLINE __forceinline
#else
    #define F_INLINE inline
#endif

#if defined(__clang__) || defined(__GNUC__)
    #define ALLOCATOR [[gnu::malloc]]
#elif defined(_MSC_VER)
    #define ALLOCATOR __declspec(restrict)
#else
    #define ALLOCATOR
#endif

#ifdef _MSC_VER
    #define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
    #define NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#define GET_DECL_OPT_OR(type, name, opt_value, ...) { auto timed = opt_value; if (!timed) [[unlikely]] { __VA_ARGS__ }; name = *timed; }
#define GET_OPT_OR(type, name, opt_value, ...) type name; GET_DECL_OPT_OR(type, name, opt_value, __VA_ARGS__)

#define ABS_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G, ...) [] () constexpr noexcept -> type { \
    COM_BOOL timed_SAC1 = requires { _BFC_G::flag; }; \
    if constexpr ( timed_SAC1 ) { \
        static_assert(!timed_SAC1 || std::same_as<std::remove_cvref_t<decltype(_BFC_G::flag)>, type>, "Invalid static flag type "#flag" (expected "#type")"); \
        return _BFC_G::flag; \
    } else { \
        __VA_ARGS__; \
    } \
}

#define STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, default_v, _BFC_G) ABS_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G, return default_v;)
#define STRICT_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G) ABS_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G, static_assert(timed_SAC1, "Static bool flag "#flag" is not declared"); return {} )

#define STATIC_FLAG_CHECK(flag, type, default_v, _BFC_G) STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, default_v, _BFC_G)()
#define STRICT_STATIC_FLAG_CHECK(flag, type, _BFC_G) STRICT_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G)()

#define FAKE_TEMPL CPP_KOSTYL template <bool = true>

#define REQ_REQ(...) requires { requires __VA_ARGS__; }

#ifndef NDEBUG
    #define dassert(expression, ...) \
        ((void)( \
            (!!(expression)) ? 0 : (macro_internal::_dassert(#expression, __FILE__, (unsigned)(__LINE__), "" __VA_OPT__(__VA_ARGS__)), 0) \
        ))
#else
    #define dassert(expression, ...) ((void) 0)
#endif

#ifndef NDEBUG
    #define ul_assert_false(...) DW0( bool timed_bl_result = MACRO_BOOL_CAST(__VA_ARGS__); dassert(timed_bl_result, "in "#__VA_ARGS__); if (!timed_bl_result) [[unlikely]] return false; )
    #define ul_assert_nullopt(...) DW0( bool timed_bl_result = MACRO_BOOL_CAST(__VA_ARGS__); dassert(timed_bl_result, "in "#__VA_ARGS__); if (!timed_bl_result) [[unlikely]] return std::nullopt; )
#else
    #define ul_assert_false(...) DW0( if (MACRO_NEGATIVE_BOOL_CAST(__VA_ARGS__)) [[unlikely]] return false; )
    #define ul_assert_nullopt(...) DW0( if (MACRO_NEGATIVE_BOOL_CAST(__VA_ARGS__)) [[unlikely]] return std::nullopt; )
#endif

#define GEN_DEF_EQ_OP(Self) bool operator==(const Self&) const = default;

#if 0
#define WIDWR_GEN_IMPLICIT_CASTS(FieldT, fieldName, SelfT) \
    IMPLICIT constexpr operator FieldT&&() && noexcept { \
        return std::move(this->fieldName); \
    } \
     \
    IMPLICIT constexpr operator FieldT&() & noexcept { \
        return this->fieldName; \
    } \
     \
    IMPLICIT constexpr operator const FieldT&() const& noexcept { \
        return this->fieldName; \
    }

#define WIDWR_GEN_IMPLICIT_CONSTR(FieldT, fieldName, SelfT) \
    IMPLICIT constexpr SelfT(const FieldT& fieldName) noexcept : fieldName(fieldName) {} \
    IMPLICIT constexpr SelfT(FieldT&& fieldName) noexcept : fieldName(std::move(fieldName)) {}

#define WIDWR_GEN_IMPLICIT_CC(FieldT, fieldName, SelfT) WIDWR_GEN_IMPLICIT_CASTS(FieldT, fieldName, SelfT) WIDWR_GEN_IMPLICIT_CONSTR(FieldT, fieldName, SelfT)
#endif

