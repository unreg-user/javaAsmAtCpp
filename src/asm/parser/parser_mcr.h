#pragma once

#include <cassert>
#include "../../cstd/macro.h"
// clang-format off

#define OR_ELSE_OPT_UNEX(type, name, opt_value, unex) OR_ELSE_OPT(type, name, opt_value, return std::unexpected{unex};)
#define OR_ELSE_OPT_UNEX_DECLARATED(type, name, opt_value, unex) OR_ELSE_OPT_DECLARATED(type, name, opt_value, return std::unexpected{unex};)

#define REQ_REQ(...) requires { requires __VA_ARGS__; }

#define DEF_BOOL_CNT_FLAG_RET(name) STRICT_STATIC_FLAG_CHECK(name, bool, ClassNT)
#define DEF_BOOL_CNT_FLAG_GEN(name) static constexpr bool name## Flag = DEF_BOOL_CNT_FLAG_RET(name)
#define DEF_CNT_FLAG_RET(name, Type) STRICT_STATIC_FLAG_CHECK(name, Type, ClassNT)
#define DEF_CNT_FLAG_GEN(name, Type) static constexpr Type name## Flag = DEF_CNT_FLAG_RET(name, Type)

#define T_CO_FLAG static constexpr bool
#define CHECK_METHOD_EXISTS_T(type, name) REQ_REQ(cstd::is_func_ptr<decltype(&type::name)>)
#define CHECK_METHOD_EXISTS(instance, name) CHECK_METHOD_EXISTS_T(decltype(instance), name)
#define CHECK_FIELD_EXISTS(instance, name) REQ_REQ(!cstd::is_func_ptr<decltype(&instance.name)>)
#define CHECK_FIELD_EXISTS_T(type, name) CHECK_FIELD_EXISTS(std::declval<type>(), name)
#define CONSTEXPR_TERN_OP(cond, ret1, ret2) [&] () -> decltype(auto) { if constexpr (cond) { return (ret1); } else { return (ret2); } } ()

#define OPT_ARRAY_HANDLE_INIT(name, uName, path, sizeT, errValidType, ...) \
        DEF_CNT_FLAG_GEN(handleWritten##uName, HandleMode); \
        T_CO_FLAG hasCorrect##uName = REQ_REQ(__VA_ARGS__); \
        T_CO_FLAG hasCorrectTranslate##uName = REQ_REQ(JaacTranslator<decltype(classNode.path)>); \
        T_CO_FLAG hasCorrectAppend##uName = hasCorrect##uName && hasCorrectTranslate##uName; \
        static_assert((handleWritten##uName##Flag == HandleMode::NONE && hasCorrectTranslate##uName) || (handleWritten##uName##Flag == HandleMode::APPEND && hasCorrectAppend##uName) || (handleWritten##uName##Flag == HandleMode::FULL && hasCorrect##uName), \
        "Invalid "#path" field type (valid requires "##errValidType##" (from asm.parser_utils, if mode != NONE) and JaacCollectionTranslatePart<TYPE> (from asm.parser_utils, if mode = APPEND)) or undeclarated"); \
        DEF_OR_ELSE_OPT_UNEX(sizeT, name##_size); \
        if constexpr (hasCorrectTranslate##uName) { classNode.path.jaac_set_source_data_interval_first(readPtr.i); classNode.path.jaac_set_source_data_size(name##_size); }\

#define DEF_OPT_ARRAY_HANDLE_INIT(name, uName, path, inElT, outElT) OPT_ARRAY_HANDLE_INIT(name, uName, path, ArraySizeT, "JaacCollection<TYPE, "#inElT", "#outElT">", JaacCollection<decltype(classNode.path), inElT, outElT>)

#define DEF_MIDDLE_END_ARRAY_HANDLE_PART(name, uName, path, ...) \
        if constexpr (handleWritten##uName##Flag != HandleMode::NONE) { \
            classNode.path.jaac_reserve(static_cast<size_t>(name##_size)); \
            if constexpr ( requires { classNode.path.jaac_start_handle(classNode); } ) classNode.path.jaac_start_handle(classNode); \
        } \
        __VA_ARGS__; \
        if constexpr (handleWritten##uName##Flag  != HandleMode::NONE && requires { classNode.path.jaac_postcomplete_handle(classNode); }) { \
            classNode.path.jaac_postcomplete_handle(classNode); \
        } \
        if constexpr (hasCorrectTranslate##uName) { classNode.path.jaac_set_source_data_interval_last(readPtr.i - 1); }

#define DEF_OPT_ARRAY_HANDLE(name, uName, path, inElT, outElT, ...) \
        DEF_OPT_ARRAY_HANDLE_INIT(name, uName, path, inElT, outElT) \
        DEF_MIDDLE_END_ARRAY_HANDLE_PART(name, uName, path, __VA_ARGS__)

#define ARRAY_SIZE_WR_BACK(name, uName, path, iswr_parse) \
        if constexpr (handleWritten##uName##Flag == HandleMode::FULL) { \
            defBackAddingLambda(classNode.path.jaac_get##iswr_parse##_size()); \
        } else if constexpr (handleWritten##uName##Flag == HandleMode::APPEND) { \
            defBackAddingLambda(classNode.path.jaac_get_source_data_size() + classNode.path.jaac_get##iswr_parse##_size()); \
        } else { \
            defBackAddingLambda(classNode.path.jaac_get_source_data_size()); \
        } \
        if constexpr (handleWritten##uName##Flag != HandleMode::FULL) { \
            outData.append_range(FileReadPtr{classNode.path.jaac_get_source_data_first(), classNode.path.jaac_get_source_data_last()}); \
        }

#define DEF_ARRAY_SIZE_WR_BACK(name, uName, path) ARRAY_SIZE_WR_BACK(name, uName, path,)
#define ARRAY_SIZE_WR_BACK_WITH_PSIZE(name, uName, path) ARRAY_SIZE_WR_BACK(name, uName, path, _parse)

#define ABS_ARRAY_BACK(name, uName, path, type, ...) \
            if constexpr (hasCorrect##uName) { \
                DEF_JAAC_FOR(name, path, type, __VA_ARGS__); \
            }

#define PRIM_ARRAY_BACK(name, uName, path) ABS_ARRAY_BACK(name, uName, path, auto, { DEF_ASSERT_UNEX(writePtr.deser_rev(i_##name)) })
#define DEF_ARRAY_BACK(name, uName, path) ABS_ARRAY_BACK(name, uName, path, auto, { DEF_ASSERT_UNEX(i_##name.back(writePtr)) })

#define DEF_PRIM_FULL_ARRAY_BACK(name, uName, path) DEF_ARRAY_SIZE_WR_BACK(name, uName, path) PRIM_ARRAY_BACK(name, uName, path)
#define DEF_FULL_ARRAY_BACK(name, uName, path) DEF_ARRAY_SIZE_WR_BACK(name, uName, path) DEF_ARRAY_BACK(name, uName, path)

#define DEF_ARRAY_FINAL_H(uName, path) \
            if constexpr (handleWritten##uName##Flag != HandleMode::NONE && requires { classNode.path.jaac_final_handle(classNode); }) { \
                classNode.path.jaac_final_handle(classNode); \
            }

#define SIMPLE_FIELD_HANDLE_INNER(name, path, Type, oldInst, timed_pre1) \
     using timed_t_0 = decltype(classNode.path); \
     T_CO_FLAG timed_b_1 = std::same_as<timed_t_0, Type>; \
     T_CO_FLAG timed_b_2 = requires { classNode.path = timed_t_0::jaac_convert_from_##name(oldInst); }; \
     T_CO_FLAG timed_s = timed_pre1 || timed_b_1 || timed_b_2; \
     if constexpr (timed_b_1) { \
         classNode.path = oldInst; \
     } else if constexpr (timed_b_2) { \
         classNode.path = timed_t_0::jaac_convert_from_##name(oldInst); \
     } else { \
         static_assert(timed_s, "Invalid type of field "#path". Flat expected "#Type); \
     }

#define FIELD_HANDLE_INNER(name, path, Type, oldInst, timed_pre1) \
    SIMPLE_FIELD_HANDLE_INNER(name, path, Type, oldInst, timed_pre1); \
    static_assert(!timed_b_2 || requires { oldInst = classNode.path.jaac_convert_to_##name(); }, "Invalid type of field "#path" (not convertable to "#name")"); \
    if constexpr (timed_b_1) { \
        return [&] () mutable -> void { oldInst = classNode.path; }; \
    } else if constexpr (timed_b_2) { \
        return [&] () mutable -> void { oldInst = classNode.path.jaac_convert_to_##name(); }; \
    } else { \
        return false; \
    }

#define SIMPLE_HANDLE(name, path, Type, oldInst) [&] { \
    T_CO_FLAG timed_pre1 = CHECK_FIELD_EXISTS(classNode, source_##name); \
    if constexpr (timed_pre1) { \
        SIMPLE_FIELD_HANDLE_INNER(source_##name, path, Type, oldInst, timed_pre1); \
    } else { \
        return false; \
    } } ()

#define HANDLE(name, path, Type, oldInst) [&] { \
    T_CO_FLAG timed_pre1 = CHECK_FIELD_EXISTS(classNode, source_##name); \
    if constexpr (timed_pre1) { \
        FIELD_HANDLE_INNER(source_##name, path, Type, oldInst, timed_pre1); \
    } else { \
        return false; \
    } } ()\

#define BACK(outDataLs, timed_lamda) \
     if constexpr (!std::same_as<decltype(timed_lamda), bool> && useWriteFlag) { \
           auto& odl = outDataLs; \
           odl.push_back(timed_lamda); \
     }

#define DEF_BACK(timed_lamda, stack) BACK(heapHandlers, timed_lamda)

#define JAAC_FOR(classNode, name, path, pre_action, _Ty, ...) \
        { \
            auto end_##name = classNode.path.jaac_end(); \
            auto begin_##name = classNode.path.jaac_begin(); \
            pre_action; \
            for (auto pre_i_##name = begin_##name; pre_i_##name != end_##name; ++pre_i_##name) { \
                  _Ty i_##name = *pre_i_##name; \
                  __VA_ARGS__;  \
            } \
        }

#define DEF_JAAC_FOR(name, path, _Ty, ...) JAAC_FOR(classNode, name, path,, _Ty, __VA_ARGS__)
#define DEF_SKIP0_JAAC_FOR(name, path, _Ty, ...) JAAC_FOR(classNode, name, path, begin_##name++, _Ty, __VA_ARGS__)

#define DEF_CHECK_METHOD_EXISTS(name) CHECK_METHOD_EXISTS(classNode, name)
#define DEF_CHECK_FIELD_EXISTS(name) CHECK_FIELD_EXISTS(classNode, name)

#define DEF_ASSERT_UNEX(bl) { bool timed_bl_result = static_cast<bool>(bl); assert(timed_bl_result); if (!timed_bl_result) [[unlikely]] return std::unexpected{ParseErr::INVALID}; }
#define DEF_ASSERT_INVALID(bl) { bool timed_bl_result = static_cast<bool>(bl); assert(timed_bl_result); if (!timed_bl_result) [[unlikely]] return ParseErr::INVALID; }

#define DEF_OR_ELSE_OPT_UNEX(type, name) OR_ELSE_OPT_UNEX(type, name, readPtr.template parse_rev<type>(), ParseErr::INVALID)
#define DEF_OR_ELSE_OPT_INVALID(type, name) OR_ELSE_OPT(type, name, readPtr.template parse_rev<type>(), return ParseErr::INVALID;)
#define DEF_CHECK_MEMORY_OR_UNEX(bytes) { bool val = readPtr.check_and_move(bytes); assert(val); if (!val) [[unlikely]] return std::unexpected{ParseErr::INVALID}; }

#define DEF_I_HANDLE(type, name) SIMPLE_HANDLE(name, name, type, name)
#define DEF_GET_AND_I_HAND(type, name) DEF_OR_ELSE_OPT_UNEX(type, name); DEF_I_HANDLE(type, name);
#define DEF_HANDLE(type, name) HANDLE(name, name, type, name)

#define DEF_FULL_HANDLE(type, name) DEF_OR_ELSE_OPT_UNEX(type, name); DEF_HANDLE(type, name); if constexpr (useWriteFlag) {defBackAddingLambda1(name)}

#define EL_CONC_WRAPPER(name) \
    struct name##ConcWrapper { \
        template <typename T> \
        T_CO_FLAG Value = name<T>; \
    };