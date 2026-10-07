#pragma once

// clang-format off

#define NODISCARD [[nodiscard]]
#define DEPRECATED(text) [[deprecated(text)]]
#define K1 template <typename> typename
#define K2 template <typename, typename> typename
#define KN template <typename...> typename
#define GETTER(name) NODISCARD constexpr auto get_##name() const noexcept {  return name; };
#define GENERIC_ALIAS(name) using name##T = name;
#define PUBLIC_GENERIC(name) GENERIC_ALIAS(name)
#define PUBLIC_GENERIC_R(name) using name = name##G;
#define GENERIC_AR_ALIAS(name) using name##TraitsT = dstd::args::TypesTraits<name>;
#define V_GENERIC_FIELD(name) static constexpr auto name##V = name;
#define PUBLIC_V_GENERIC(name) V_GENERIC_FIELD(name)
#define PUBLIC_V_GENERIC_R(name) static constexpr auto name = name##G;
#define UNION_CONSTR(uName, fieldName) uName(decltype(fieldName) fieldName) noexcept : fieldName(fieldName) {};
#define UNION_CONSTR_EXPLTYPE(uName, fieldType, fieldName) uName(fieldType fieldName) noexcept : fieldName(fieldName) {};
#define DEL_CPY(tName) \
        tName(const tName&) = delete; \
        tName& operator=(const tName&) = delete;

#define F_INLINE __attribute__((always_inline))
#define ALLOCATOR __attribute__((malloc))

#define CSFLAG static constexpr bool
#define OR_ELSE_OPT_DECLARATED(type, name, opt_value, ...) do { auto timed = opt_value; if (!timed) [[unlikely]] { __VA_ARGS__ }; name = *timed; } while (0)
#define OR_ELSE_OPT(type, name, opt_value, ...) type name; OR_ELSE_OPT_DECLARATED(type, name, opt_value, __VA_ARGS__)
#define UNUSED [[maybe_unused]]
#define DISCARD (void)

#define ABS_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G, ...) [] constexpr noexcept -> type { \
    if constexpr ( requires { _BFC_G::flag; } ) { \
        static_assert(std::same_as<std::remove_cvref_t<decltype(_BFC_G::flag)>, type>, "Invalid static flag type "#flag" (expected "#type")"); \
        return _BFC_G::flag; \
    } else { \
        __VA_ARGS__; \
    } \
}

#define STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, default_v, _BFC_G) ABS_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G, return default_v;)
#define STRICT_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G) ABS_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G, static_assert(!requires { _BFC_G::flag; }, "Static bool flag "#flag" is not declarated"))

#define STATIC_FLAG_CHECK(flag, type, default_v, _BFC_G) STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, default_v, _BFC_G)()
#define STRICT_STATIC_FLAG_CHECK(flag, type, _BFC_G) STRICT_STATIC_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, type, _BFC_G)()

/*
#include <utility>

#ifdef NDEBUG
    #define cassert(expression) ((!expression) ? std::unreachable() : (void)0)
#else
    #define cassert(expression) assert(expression);
#endif
*/


