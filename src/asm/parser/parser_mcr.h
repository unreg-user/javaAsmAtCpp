#pragma once

#include <cassert>
#include "../../cstd/macro.h"
// clang-format off

#define OR_ELSE_OPT_UNEX(type, name, opt_value, unex) OR_ELSE_OPT(type, name, opt_value, return std::unexpected{unex};)
#define OR_ELSE_OPT_UNEX_DECLARED(type, name, opt_value, unex) OR_ELSE_OPT_DECLARED(type, name, opt_value, return std::unexpected{unex};)

#define COM_BOOL COM bool

// legacy
#define DEF_BOOL_CNT_FLAG(name) STRICT_STATIC_FLAG_CHECK(name, bool, ClassNT)
#define DEF_BOOL_CNT_FLAG_GEN(name) COM_BOOL name## Flag = DEF_BOOL_CNT_FLAG(name)
#define DEF_CNT_FLAG(name, Type) STRICT_STATIC_FLAG_CHECK(name, Type, ClassNT)
#define DEF_CNT_FLAG_GEN(name, Type) COM Type name## Flag = DEF_CNT_FLAG(name, Type)

#define DEF_FLAG_AT(name, type, defValue, ctx) STATIC_FLAG_CHECK(name, type, defValue, ctx)
#define DEF_FLAG_AT_GEN(name, type, defValue, ctx) COM type name##Flag = DEF_FLAG_AT(name, type, defValue, ctx)
#define DEF_BOOL_FLAG_AT(name, defValue, ctx) DEF_FLAG_AT(name, bool, defValue, ctx)
#define DEF_BOOL_FLAG_AT_GEN(name, defValue, ctx) DEF_FLAG_AT_GEN(name, bool, defValue, ctx)

#define DEF_STRICT_FLAG_AT(name, type, ctx) STRICT_STATIC_FLAG_CHECK(name, type, ctx)
#define DEF_STRICT_FLAG_AT_GEN(name, type, ctx) COM type name##Flag = DEF_STRICT_FLAG_AT(name, type, ctx)
#define DEF_STRICT_BOOL_FLAG_AT(name, ctx) DEF_STRICT_FLAG_AT(name, bool, ctx)
#define DEF_STRICT_BOOL_FLAG_AT_GEN(name, ctx) DEF_STRICT_FLAG_AT_GEN(name, bool, ctx)

#define CHECK_METHOD_EXISTS_T(type, name) REQ_REQ(cstd::is_func_ptr<decltype(&type::name)>)
#define CHECK_METHOD_EXISTS(instance, name) CHECK_METHOD_EXISTS_T(decltype(instance), name)
#define CHECK_FIELD_EXISTS(instance, name) REQ_REQ(!cstd::is_func_ptr<decltype(&instance.name)>)
#define CHECK_FIELD_EXISTS_T(type, name) CHECK_FIELD_EXISTS(std::declval<type>(), name)

#define CONSTEXPR_TERN_OP(cond, ret1, ret2) [&] () -> decltype(auto) { if constexpr (cond) { return (ret1); } else { return (ret2); } } ()

#define COM_ARRAY_HANDLE(uName, path, errTxt, ...) \
        DEF_CNT_FLAG_GEN(handleMode##uName, HandleMode); \
        COM_BOOL hasCorrect##uName = REQ_REQ(__VA_ARGS__); \
        COM_BOOL shouldHasCorrect##uName = (handleMode##uName##Flag == HandleMode::FULL); \
        static_assert(!shouldHasCorrect##uName || hasCorrect##uName, errTxt); \
        COM_BOOL hasCorrectTranslate##uName = REQ_REQ(JaacTranslator<decltype(classNode.path)>);

#define OPT_ARRAY_HANDLE_INIT(name, uName, path, sizeT) \
        DEF_OR_ELSE_OPT_UNEX(sizeT, name##_size); \
        if constexpr (hasCorrectTranslate##uName) { classNode.path.jaac_set_source_data_interval_begin(readPtr.i); classNode.path.jaac_set_source_data_size(name##_size); }\

#define DEF_OPT_ARRAY_HANDLE_INIT(name, uName, path) OPT_ARRAY_HANDLE_INIT(name, uName, path, wrappers::ArraySizeT)

#define DEF_COM_ARRAY_HANDLE(uName, path, inElT) COM_ARRAY_HANDLE(uName, path, "Invalid "#path" field type (valid requires JaacCollection<., "#inElT"> (from asm.parser_concepts, if mode == FULL))", JaacCollection<decltype(classNode.path), inElT>)

#define DEF_MIDDLE_END_ARRAY_HANDLE_PART(name, uName, path, ...) \
        if constexpr (handleMode##uName##Flag != HandleMode::NONE) { \
            classNode.path.jaac_reserve(static_cast<size_t>(name##_size)); \
            if constexpr ( requires { classNode.path.jaac_start_handle(classNode); } ) classNode.path.jaac_start_handle(classNode); \
        } \
        __VA_ARGS__; \
        if constexpr (handleMode##uName##Flag != HandleMode::NONE && requires { classNode.path.jaac_postcomplete_handle(classNode); }) { \
            classNode.path.jaac_postcomplete_handle(classNode); \
        } \
        if constexpr (hasCorrectTranslate##uName) { classNode.path.jaac_set_source_data_interval_end(readPtr.i); }

#define DEF_DESER(uName, path, iterCowr, hasParseSizeAndSkip0, sizeT, ...) deser_array<iterCowr, handleMode##uName##Flag, sizeT, hasParseSizeAndSkip0, SpecR, SpecW>(writePtr, classNode.path, __VA_ARGS__)
#define DEF_JAAC_PRIM_DESER(uName, path, argT, sizeT) DEF_DESER(uName, path, cstd::ImplConvertableCowr<argT>, false, sizeT, [&writePtr] (argT value) { value.deser(writePtr); })
#define DEF_JAAC_COMPLEX_DESER(uName, path, sizeT) DEF_DESER(uName, path, JaacDeserableCowr<SpecW>, false, sizeT, [&writePtr] (auto& value) { value.deser(writePtr); })
#define DEF_JAAC_COMPLEX_DESER_SKIP0(uName, path, sizeT) DEF_DESER(uName, path, JaacDeserableCowr<SpecW>, true, sizeT, [&writePtr] (auto& value) { value.deser(writePtr); })

#define DEF_OPT_ARRAY_HANDLE(name, uName, path, ...) \
        DEF_OPT_ARRAY_HANDLE_INIT(name, uName, path) \
        DEF_MIDDLE_END_ARRAY_HANDLE_PART(name, uName, path, __VA_ARGS__)

#define DEF_ARRAY_PREWRITE_CALL(uName, path) \
            if constexpr (handleMode##uName##Flag != HandleMode::NONE && requires { classNode.path.jaac_prewrite_handle(classNode); }) { \
                classNode.path.jaac_prewrite_handle(classNode); \
            }

#define FIELD_WRITER_INNER(name, path, Type, oldInst, timed_pre1) \
    using timed_t_1 = decltype(classNode.path); \
    COM_BOOL timed_b_1 = cstd::is_implicit_convertable<Type, timed_t_1>; \
    COM_BOOL timed_b_2 = requires { { timed_t_1::jaac_convert_from_##name(oldInst) } -> std::same_as<timed_t_1>; }; \
    COM_BOOL timed_s = !timed_pre1 || timed_b_1 || timed_b_2; \
    if constexpr (timed_b_1) { \
        classNode.path = oldInst; \
    } else if constexpr (timed_b_2) { \
        classNode.path = timed_t_1::jaac_convert_from_##name(oldInst); \
    } else { \
        static_assert(timed_s, "Invalid type of field "#path". Flat expected "#Type); \
    }

#define FIELD_RETURNER_INNER(name, path, Type, oldInst, timed_pre1) \
    using timed_t_1 = decltype(classNode.path); \
    COM_BOOL timed_b_1 = cstd::is_implicit_convertable<timed_t_1, Type>; \
    COM_BOOL timed_b_2 = requires { { classNode.path.jaac_convert_to_##name() } -> std::same_as<Type>; }; \
    COM_BOOL timed_s = !timed_pre1 || timed_b_1 || timed_b_2; \
    if constexpr (timed_b_1) { \
        return [&] () mutable -> void { oldInst = classNode.path; }; \
    } else if constexpr (timed_b_2) { \
        return [&] () mutable -> void { oldInst = classNode.path.jaac_convert_to_##name(); }; \
    } else { \
        static_assert(timed_s, "Invalid type of field "#path". Flat expected "#Type); \
        return false; \
    }

#define FIELD_HANDLE_OUTER(path, ...) [&] { \
    COM_BOOL timed_pre1 = CHECK_FIELD_EXISTS(classNode, path); \
    if constexpr (timed_pre1) { \
        __VA_ARGS__; \
    } else { \
        return false; \
    } } ()

#define FIELD_WRITER(name, path, Type, oldInst) FIELD_HANDLE_OUTER(path, FIELD_WRITER_INNER(name, path, Type, oldInst, timed_pre1))

#define FIELD_RETURNER(name, path, Type, oldInst) FIELD_HANDLE_OUTER(path, FIELD_RETURNER_INNER(name, path, Type, oldInst, timed_pre1))

#define DEF_CHECK_METHOD_EXISTS(name) CHECK_METHOD_EXISTS(classNode, name)
#define DEF_CHECK_FIELD_EXISTS(name) CHECK_FIELD_EXISTS(classNode, name)

#define DEF_ASSERT_UNEX(...) { bool timed_bl_result = static_cast<bool>(__VA_ARGS__); dassert(timed_bl_result, "in "#__VA_ARGS__); if (!timed_bl_result) [[unlikely]] return std::unexpected{ParseErr::INVALID}; }
#define DEF_ASSERT_INVALID(...) { bool timed_bl_result = static_cast<bool>(__VA_ARGS__); dassert(timed_bl_result, "in "#__VA_ARGS__); if (!timed_bl_result) [[unlikely]] return ParseErr::INVALID; }
#define DEF_ASSERT_FALSE(...) { bool timed_bl_result = static_cast<bool>(__VA_ARGS__); dassert(timed_bl_result, "in "#__VA_ARGS__); if (!timed_bl_result) [[unlikely]] return false; }

#define DEF_OR_ELSE_OPT_UNEX(type, name) OR_ELSE_OPT_UNEX(type, name, type::parse(readPtr), ParseErr::INVALID)
#define DEF_EXISTS_OR_ELSE_OPT_UNEX(type, name) { auto timed = type::parse(readPtr); if (!timed) [[unlikely]] { return std::unexpected{ParseErr::INVALID}; }; name = *timed; }
#define DEF_OR_ELSE_OPT_INVALID(type, name) OR_ELSE_OPT(type, name, type::parse(readPtr), return ParseErr::INVALID;)
#define DEF_OR_ELSE_RET_FALSE(type, name) OR_ELSE_OPT(type, name, type::parse(readPtr), return false;)
#define DEF_CHECK_MEMORY_OR_UNEX(bytes) { bool val = readPtr.check_and_move(bytes); assert(val); if (!val) [[unlikely]] return std::unexpected{ParseErr::INVALID}; }

#define DEF_FIELD_WRITER(type, name) FIELD_WRITER(name, name, type, name)
#define DEF_EXISTS_GET_AND_FIELD_WRITE(type, name) DEF_EXISTS_OR_ELSE_OPT_UNEX(type, name); DEF_FIELD_WRITER(type, name);
#define DEF_GET_AND_FIELD_WRITE(type, name) DEF_OR_ELSE_OPT_UNEX(type, name); DEF_FIELD_WRITER(type, name);
#define DEF_FIELD_RETURNER(type, name) FIELD_RETURNER(name, name, type, name)