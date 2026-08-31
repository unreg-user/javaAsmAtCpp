#pragma once

// clang-format off

#define OR_ELSE_OPT_UNEX(type, name, opt_value, unex) OR_ELSE_OPT(type, name, opt_value, return std::unexpected{unex};)

#define HANDLE(name, Type, oldInst, classNode, useWriteFlag) \
do { \
if constexpr ( requires { classNode.handle_##name(oldInst); } ) { \
    using timed_t_0 = decltype((classNode.handle_##name(oldInst))); \
    static constexpr bool timed_b_1 = requires { { classNode.handle_##name(oldInst) } -> std::same_as<Type>; }; \
    static constexpr bool timed_b_2 = requires { { classNode.handle_##name(oldInst) } -> std::same_as<void>; };\
    static constexpr bool timed_b_3 = requires { { std::declval<timed_t_0>().jaac_convert_to_##name() } -> std::same_as<Type>; }; \
    static constexpr bool timed_s = timed_b_1 || timed_b_2 || timed_b_3; \
    if constexpr (useWriteFlag) { \
        if constexpr (timed_b_1) { \
            oldInst = classNode.handle_##name(oldInst); \
        } else if constexpr (timed_b_2) { \
            classNode.handle_##name(oldInst); \
        } else if constexpr (timed_b_3) { \
            oldInst = classNode.handle_##name(oldInst).jaac_convert_to_##name(); \
        } \
    } else if constexpr (timed_s) { \
        classNode.handle_##name(oldInst); \
    } else { \
        static_assert(timed_s, "Invalid return type in method handle_"#name". Flat expected "#Type); \
    } \
} else if constexpr ( requires { classNode.name; } ) { \
    using timed_t_0 = decltype(classNode.name); \
    static constexpr bool timed_b_1 = std::same_as<timed_t_0, uint32_t>; \
    static constexpr bool timed_b_2 = requires { classNode.name = timed_t_0::jaac_convert_from_##name(oldInst); }; \
    static constexpr bool timed_s = timed_b_1 || timed_b_2; \
    if constexpr (timed_b_1) { \
        classNode.name = oldInst; \
    } else if constexpr (timed_b_2) { \
        classNode.name = timed_t_0::jaac_convert_from_##name(oldInst); \
    } else { \
        static_assert(timed_s, "Invalid type of field "#name". Flat expected "#Type); \
    } \
} \
} while (0)

#define HANDLE_MU_LAMBDA(name, classNode, useWriteFlag, ...) \
     [&] <typename Callable> (Callable&& callable = [] () noexcept {}) noexcept { \
         if constexpr ( requires { classNode.m_handle_##name(__VA_ARGS__); } ) { \
             auto timed = [&classNode] (auto... args) { \
                static const auto timed_l_1 = [] (auto value) { auto [__VA_ARGS__] = value; }; \
                static constexpr bool timed_b_1 = requires { timed_l_1(classNode.m_handle_##name(args...)); }; \
                static constexpr bool timed_b_2 = requires { { classNode.m_handle_##name(args...) } -> std::same_as<void>; };\
                static constexpr bool timed_b_3 = requires { timed_l_1(classNode.m_handle_##name(args...).jaac_convert_to_m_##name()); }; \
                static constexpr bool timed_s = timed_b_1 || timed_b_2 || timed_b_3; \
                if constexpr (useWriteFlag) { \
                    if constexpr (timed_b_1) { \
                        auto [__VA_ARGS__] = classNode.m_handle_##name(args...); \
                        return std::tuple(__VA_ARGS__); \
                    } else if constexpr (timed_b_2) { \
                        classNode.m_handle_##name(args...); \
                        return false; \
                    } else if constexpr (timed_b_3) { \
                        auto [__VA_ARGS__] = classNode.m_handle_##name(args...).jaac_convert_to_m_##name(); \
                        return std::tuple(__VA_ARGS__); \
                    } \
                } else if constexpr (timed_s) { \
                    classNode.m_handle_##name(args...); \
                       return false; \
                } else { \
                    static_assert(timed_s, "Invalid return type in method m_handle_"#name); \
                }\
             }(__VA_ARGS__);    \
             if constexpr (!std::same_as<decltype(timed), bool>) { \
                std::tie(__VA_ARGS__) = timed; \
             } \
         } else if constexpr ( requires { classNode.m_handle_##name({__VA_ARGS__}); } ) { \
             auto timed = [&classNode] (auto... args) { \
                static const auto timed_l_1 = [] (auto value) { auto [__VA_ARGS__] = value; }; \
                static constexpr bool timed_b_1 = requires { timed_l_1(classNode.m_handle_##name({args...})); }; \
                static constexpr bool timed_b_2 = requires { { classNode.m_handle_##name({args...}) } -> std::same_as<void>; };\
                static constexpr bool timed_b_3 = requires { timed_l_1(classNode.m_handle_##name({args...}).jaac_convert_to_m_##name()); }; \
                static constexpr bool timed_s = timed_b_1 || timed_b_2 || timed_b_3; \
                if constexpr (useWriteFlag) { \
                    if constexpr (timed_b_1) { \
                        auto [__VA_ARGS__] = classNode.m_handle_##name({args...}); \
                        return std::tuple(__VA_ARGS__); \
                    } else if constexpr (timed_b_2) { \
                        classNode.m_handle_##name({args...}); \
                        return false; \
                    } else if constexpr (timed_b_3) { \
                        auto [__VA_ARGS__] = classNode.m_handle_##name({args...}).jaac_convert_to_m_##name(); \
                        return std::tuple(__VA_ARGS__); \
                    } \
                } else if constexpr (timed_s) { \
                    classNode.m_handle_##name({args...}); \
                    return false; \
                } else { \
                    static_assert(timed_s, "Invalid return type in method m_handle_"#name); \
                }\
             }(__VA_ARGS__);    \
             if constexpr (!std::same_as<decltype(timed), bool>) { \
                std::tie(__VA_ARGS__) = timed; \
             } \
         } else { \
            callable(); \
         }\
     }

#define DEF_HANDLE_MU_LAMBDA(name, ...) HANDLE_MU_LAMBDA(name, classNode, useWriteFlag, __VA_ARGS__)

#define DEF_ASSERT_UNEX(bl) { bool timed_bl_result = static_cast<bool>(bl); assert(timed_bl_result); if (!timed_bl_result) [[unlikely]] return std::unexpected{ParseErr::INVALID}; }

#define DEF_OR_ELSE_OPT_UNEX(type, name) OR_ELSE_OPT_UNEX(type, name, symPtr.template memcpy_get_rev<type>(), ParseErr::INVALID)
#define DEF_HANDLE(type, name) HANDLE(name, type, name, classNode, useWriteFlag)

#define DEF_FULL_HANDLE(type, name) DEF_OR_ELSE_OPT_UNEX(type, name); DEF_HANDLE(type, name); if constexpr (useWriteFlag) {defBackAddingLambda1(name)}

#define DEF_STATIC_BOOL_CNT_FLAG_RET(name, default_v) STATIC_BOOL_FLAG_CHECK(name, default_v, ClassNT)
#define DEF_STATIC_BOOL_CNT_FLAG_GEN(name, default_v) static constexpr bool name## Flag = DEF_STATIC_BOOL_CNT_FLAG_RET(name, default_v)