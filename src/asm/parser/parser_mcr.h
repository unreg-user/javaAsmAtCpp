#pragma once

#include <cassert>
#include "../../cstd/macro.h"
// clang-format off

#define USE_VOLATILE 0
#define COMMON_ENUM_INNER(ENUM, at) ENUM{ENUM::At::at}
#define UNEX_INNER(at) COMMON_ENUM_INNER(ParseErr, at)
#define UNEX(at) std::unexpected{UNEX_INNER(at)}
#define EX_INNER(at) COMMON_ENUM_INNER(DeserErr, at)

#ifndef NDEBUG
    #define DEF_ASSERT_UNEX(at, ...) DW0( bool timed_bl_result = MACRO_BOOL_CAST(__VA_ARGS__); dassert(timed_bl_result, "in "#__VA_ARGS__); if (!timed_bl_result) [[unlikely]] return UNEX(at); )
    #define DEF_ASSERT_EX(at, ...) DW0( bool timed_bl_result = MACRO_BOOL_CAST(__VA_ARGS__); dassert(timed_bl_result, "in "#__VA_ARGS__); if (!timed_bl_result) [[unlikely]] retL(EX_INNER(at)); )
#else
    #define DEF_ASSERT_UNEX(at, ...) DW0( if (MACRO_NEGATIVE_BOOL_CAST(__VA_ARGS__)) [[unlikely]] return UNEX(at); )
    #define DEF_ASSERT_EX(at, ...) DW0( if (MACRO_NEGATIVE_BOOL_CAST(__VA_ARGS__)) [[unlikely]] return retL(EX_INNER(at)); )
#endif

#define ABS_PARSE(type, name, ...) DW0( auto timed = type::parse(readPtr); __VA_ARGS__; name = std::move(*timed); )
#define ABS_UNWR_PARSE(type, name, ...) DW0( auto timed = type::parse(readPtr); __VA_ARGS__; name = gstd::unwrap(std::move(*timed)); )
#define UNWR_OF(type) gstd::GetWidTypeOrVoid<type>

#define PARSE_EXISTS_OR_RET_UNEX(type, name, at) ABS_PARSE(type, name, DEF_ASSERT_UNEX(at, timed))
#define PARSE_EXISTS_OR_RET_FALSE(type, name) ABS_PARSE(type, name, ul_assert_false(timed))
#define PARSE_EXISTS_OR_RET_NULLOPT(type, name) ABS_PARSE(type, name, ul_assert_nullopt(timed))

#define PARSE_N_UNWRAP_EXISTS_OR_RET_UNEX(type, name, at) ABS_UNWR_PARSE(type, name, DEF_ASSERT_UNEX(at, timed))
#define PARSE_N_UNWRAP_EXISTS_OR_RET_FALSE(type, name) ABS_UNWR_PARSE(type, name, ul_assert_false(timed))
#define PARSE_N_UNWRAP_EXISTS_OR_RET_NULLOPT(type, name) ABS_UNWR_PARSE(type, name, ul_assert_nullopt(timed))

#define PARSE_OR_RET_UNEX(type, name, at) type name; PARSE_EXISTS_OR_RET_UNEX(type, name, at)
#define PARSE_OR_RET_FALSE(type, name) type name; PARSE_EXISTS_OR_RET_FALSE(type, name)
#define PARSE_OR_RET_NULLOPT(type, name) type name; PARSE_EXISTS_OR_RET_NULLOPT(type, name)

#define PARSE_N_UNWRAP_OR_RET_UNEX(type, name, at) UNWR_OF(type) name; PARSE_N_UNWRAP_EXISTS_OR_RET_UNEX(type, name, at)
#define PARSE_N_UNWRAP_OR_RET_FALSE(type, name) UNWR_OF(type) name; PARSE_N_UNWRAP_EXISTS_OR_RET_FALSE(type, name)
#define PARSE_N_UNWRAP_OR_RET_NULLOPT(type, name) UNWR_OF(type) name; PARSE_N_UNWRAP_EXISTS_OR_RET_NULLOPT(type, name)


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
#define CHECK_FIELD_EXISTS_T(type, name) requires (type instance) { requires CHECK_FIELD_EXISTS(instance, name); }

#define CONSTEXPR_TERN_OP(cond, ret1, ret2) [&] () -> decltype(auto) { if constexpr (cond) { return (ret1); } else { return (ret2); } } ()

#define COM_ARRAY_HANDLE(SAC, uName, path, errTxt, ...) \
        DEF_CNT_FLAG_GEN(handleMode##uName, HandleMode); \
        COM_BOOL hasCorrect##uName = REQ_REQ(__VA_ARGS__); \
        COM_BOOL shouldHasCorrect##uName = (handleMode##uName##Flag == HandleMode::FULL); \
        static_assert(!SAC || (!shouldHasCorrect##uName || hasCorrect##uName), errTxt); \
        COM_BOOL hasCorrectTranslate##uName = REQ_REQ(JaacTranslator<decltype(classNode.path)>);

#define OPT_ARRAY_HANDLE_INIT(at, name, uName, path, sizeT) \
        PARSE_N_UNWRAP_OR_RET_UNEX(sizeT, name##Size, at); \
        if constexpr (hasCorrectTranslate##uName) { classNode.path.jaac_set_source_data_interval_begin(readPtr.i); classNode.path.jaac_set_source_data_size(static_cast<size_t>(name##Size)); }\

#define DEF_OPT_ARRAY_HANDLE_INIT(at, name, uName, path) OPT_ARRAY_HANDLE_INIT(at, name, uName, path, wrappers::ArraySizeT)

#define DEF_COM_ARRAY_HANDLE(SAC, uName, path, inElT) COM_ARRAY_HANDLE(SAC, uName, path, "Invalid "#path" field type (valid requires JaacCollection<., "#inElT"> (from asm.parser_concepts, if mode == FULL))", JaacCollection<decltype(classNode.path), inElT>)
#define LENGTH_BOOL_GEN(S_NAME) COM_BOOL contains##S_NAME##_Parsing = parseLengthFlag >= ParseLength::S_NAME;

#define DEF_ARR_POSTCOMPLETE_HANDLE(at, path) \
        if constexpr ( requires { { classNode.path.jaac_postcomplete_handle(classNode) } -> gstd::bool_or_true; } ) { \
            DEF_ASSERT_UNEX(at, classNode.path.jaac_postcomplete_handle(classNode)); \
        } \

#define DEF_MANUAL_ARR_HANDLE(at, path) \
    PARSE_EXISTS_OR_RET_UNEX(decltype(classNode.path), classNode.path, at); \
    DEF_ARR_POSTCOMPLETE_HANDLE(at, path);

#define DEF_MIDDLE_END_ARRAY_HANDLE_PART(at, name, uName, path, ...) \
        if constexpr (handleMode##uName##Flag != HandleMode::NONE) { \
            classNode.path.jaac_reserve(static_cast<size_t>(name##Size)); \
            if constexpr ( requires { { classNode.path.jaac_start_handle(classNode) } -> gstd::bool_or_true; } ) DEF_ASSERT_UNEX(at, classNode.path.jaac_start_handle(classNode)); \
        } \
        __VA_ARGS__; \
        DEF_ARR_POSTCOMPLETE_HANDLE(at, path) \
        if constexpr (hasCorrectTranslate##uName) { classNode.path.jaac_set_source_data_interval_end(readPtr.i); }

#define DEF_DESER(at, uName, path, iterCowr, hasParseSizeAndSkip0, sizeT, ...) DEF_ASSERT_EX(at, deser_array<iterCowr, handleMode##uName##Flag, sizeT, hasParseSizeAndSkip0, SpecR, SpecW>(writePtr, classNode.path, __VA_ARGS__))
#define DEF_JAAC_PRIM_DESER(at, uName, path, argT, sizeT) DEF_DESER(at, uName, path, cstd::ImplConvertableCowr<argT>, false, sizeT, [&writePtr] (argT value) { value.deser(writePtr); })
#define DEF_JAAC_COMPLEX_DESER(at, uName, path, sizeT) DEF_DESER(at, uName, path, JaacDeserableCowr<SpecW>, false, sizeT, [&writePtr] (auto& value) { value.deser(writePtr); })
#define DEF_JAAC_COMPLEX_DESER_SKIP0(at, uName, path, sizeT) DEF_DESER(at, uName, path, JaacDeserableCowr<SpecW>, true, sizeT, [&writePtr] (auto& value) { value.deser(writePtr); })

#define DEF_OPT_ARRAY_HANDLE(at, name, uName, path, ...) \
        DEF_OPT_ARRAY_HANDLE_INIT(at, name, uName, path) \
        DEF_MIDDLE_END_ARRAY_HANDLE_PART(at, name, uName, path, __VA_ARGS__)

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

#define FIELD_HANDLE_OUTER(SAC, path, ...) [&] { \
    COM_BOOL timed_pre1 = SAC && CHECK_FIELD_EXISTS(classNode, path); \
    if constexpr (timed_pre1) { \
        __VA_ARGS__; \
    } else { \
        return false; \
    } } ()

#define FIELD_WRITER(SAC, name, path, Type, oldInst) FIELD_HANDLE_OUTER(SAC, path, FIELD_WRITER_INNER(name, path, Type, oldInst, timed_pre1))

#define FIELD_RETURNER(SAC, name, path, Type, oldInst) FIELD_HANDLE_OUTER(SAC, path, FIELD_RETURNER_INNER(name, path, Type, oldInst, timed_pre1))

#define DEF_CHECK_METHOD_EXISTS(name) CHECK_METHOD_EXISTS(classNode, name)
#define DEF_CHECK_FIELD_EXISTS(name) CHECK_FIELD_EXISTS(classNode, name)

#define DEF_FIELD_WRITER(SAC, type, name) FIELD_WRITER(SAC, name, name, type, name)
#define DEF_EXISTS_GET_AND_FIELD_WRITE(at, SAC, type, name) PARSE_EXISTS_OR_RET_UNEX(type, name, at); DEF_FIELD_WRITER(SAC, type, name);
#define DEF_GET_AND_FIELD_WRITE(SAC, at, type, name) PARSE_OR_RET_UNEX(type, name, at); DEF_FIELD_WRITER(SAC, type, name);
#define DEF_FIELD_RETURNER(SAC, type, name) FIELD_RETURNER(SAC, name, name, type, name)