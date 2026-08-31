#pragma once

// clang-format off

#define NODISCARD [[nodiscard]]
#define DEPRECATED(text) [[deprecated(text)]]
#define K1 template <typename> typename
#define K2 template <typename, typename> typename
#define KN template <typename...> typename
#define GETTER(name) NODISCARD constexpr auto get_##name() const noexcept {  return name; };
#define GENERIC_ALIAS(name) using name##T = name;
#define V_GENERIC_FIELD(name) static constexpr auto name##V = name;
#define UNION_CONSTR(uName, fieldName) uName(decltype(fieldName) fieldName) noexcept : fieldName(fieldName) {};
#define UNION_CONSTR_EXPLTYPE(uName, fieldType, fieldName) uName(fieldType fieldName) noexcept : fieldName(fieldName) {};
#define DEL_CPY(tName) \
        tName(const tName&) = delete; \
        tName& operator=(const tName&) = delete;

#define ALLOCATOR __attribute__((malloc))
#define F_INLINE __forceinline
#define CSFLAG static constexpr bool
#define OR_ELSE_OPT(type, name, opt_value, ...) type name; do { auto timed = opt_value; if (!timed) [[unlikely]] { __VA_ARGS__ }; name = *timed; } while (0)
#define UNUSED [[maybe_unused]]
#define DISCARD (void)


#define STATIC_BOOL_FLAG_CHECKER_LAMBDA(flag, default_v) [] <typename _BFC_G>() constexpr noexcept -> bool { \
    if constexpr ( requires { _BFC_G::flag; } ) { \
        static_assert(requires { { _BFC_G::flag } -> std::same_as<bool>; }, "Invalid static bool flag type "#flag); \
        return _BFC_G::flag; \
    } else { \
        return default_v; \
    } \
}

#define STATIC_BOOL_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, default_v, _BFC_G) [] constexpr noexcept -> bool { \
    if constexpr ( requires { _BFC_G::flag; } ) { \
        static_assert(std::same_as<decltype(_BFC_G::flag), const bool>, "Invalid static bool flag type "#flag); \
        return _BFC_G::flag; \
    } else { \
        return default_v; \
    } \
}

#define STATIC_BOOL_FLAG_CHECK(flag, default_v, _BFC_G) STATIC_BOOL_FLAG_CHECKER_LAMBDA_WITH_TYPE(flag, default_v, _BFC_G)()

/*
#include <utility>

#ifdef NDEBUG
    #define cassert(expression) ((!expression) ? std::unreachable() : (void)0)
#else
    #define cassert(expression) assert(expression);
#endif
*/


