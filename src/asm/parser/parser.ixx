module;

#include <concepts>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <type_traits>
#include <vector>

#include "parser_mcr.h"

export module parser;

import macro_u;
import reader;
import opcodes;
import compl_flags;
import simple_cstd;
import gstd;
import cp_consts;
import parser_utils;
import parser_concepts;
import array_parsers;
import dstd;
import parser_mxns;
import parser_base;
import parser_widstructs;
import costd;

/**
 * According to the specification, if a collection in classNode is a JaacTranslator, the invocation of its
 * (JaacTranslator) methods is guaranteed; if it is a JaacCollection, the invocation of its (JaacCollection) methods is
 * not guaranteed.
 */
export namespace parser {
    template <typename ClassNT>
    concept IsConvertableAtReturn = requires(ClassNT classNode) { auto(classNode.jaac_return_convert()); };

    template <typename ClassNT>
    using ClassNTAtRet = decltype(([] {
        if constexpr (IsConvertableAtReturn<ClassNT>) { // NOLINT(*-branch-clone)
            return dstd::args::TypeWrapper<decltype((std::declval<ClassNT>().jaac_return_convert()))>{};
        } else {
            return dstd::args::TypeWrapper<ClassNT>{};
        }
    }()))::InstanceT;

    template <typename ClassNT>
    using CondPairInParseRetType =
            gstd::cond_pair<ClassNTAtRet<ClassNT>, std::conditional_t<UseWriteConcept<ClassNT>, DeserErr,
                                                                      gstd::ccompl_value<DeserErr, DeserErr{}>>>;

    template <typename ClassNT>
    using ParseRetType = std::expected<CondPairInParseRetType<JaacClassNTRetConverted<ClassNT>>, ParseErr>;

    template <typename ClassNT /*CNT*/, typename SpecR = cflags::class_file_ptr,
              typename SpecW = cflags::class_file_ptr, bool checkMagic = true>
    ParseRetType<ClassNT> safe_parse(std::span<uint8_t> inputData, std::vector<uint8_t>* outputPtr = nullptr) {
        static_assert(JaacClassNT<ClassNT>);

        using ReadPtr = ReadPtrBySpec<SpecR>;
        using WritePtr = WritePtrBySpec<SpecW>;

        ClassNT classNode{};
        DEF_BOOL_CNT_FLAG_GEN(useWrite);
        DEF_CNT_FLAG_GEN(parseLength, ParseLength);
        LENGTH_BOOL_GEN(CPOOL);
        LENGTH_BOOL_GEN(IFACES);
        LENGTH_BOOL_GEN(FIELDS);
        LENGTH_BOOL_GEN(METHODS);
        LENGTH_BOOL_GEN(FULL);

        ReadPtr readPtr{inputData};

        if constexpr (checkMagic) {
            PARSE_OR_RET_UNEX(wrappers::MagicNumber, magic, MAGIC);
            DEF_ASSERT_UNEX(MAGIC, static_cast<uint32_t>(magic) == opcodes::magic);
        } else {
            DEF_ASSERT_UNEX(MAGIC, readPtr.template check_and_move<4>());
        }

        DEF_GET_AND_FIELD_WRITE(true, VERSION, wrappers::VersionNumber, version);


        DEF_BOOL_CNT_FLAG_GEN(parseCpoolStructure);
        using CpoolArrInputT = std::conditional_t<parseCpoolStructureFlag, cp_consts::traits::cp_var, u8_span>;

        DEF_COM_ARRAY_HANDLE(containsCPOOL_Parsing, Cpool, cpool, CpoolArrInputT);
        DEF_COM_ARRAY_HANDLE(containsIFACES_Parsing, Ifaces, ifaces, wrappers::CpoolSizeT);
        DEF_COM_ARRAY_HANDLE(containsFIELDS_Parsing, Fields, fields, common::MFInfo<>);
        DEF_COM_ARRAY_HANDLE(containsMETHODS_Parsing, Methods, methods, common::MFInfo<>);
        DEF_COM_ARRAY_HANDLE(containsFULL_Parsing, Attrs, attrs, common::DefVeryFullAttribute);


        wrappers::AccessFlags accessFlags;
        wrappers::CpoolSizeT thisClass;
        wrappers::CpoolSizeT superClass;

        if constexpr (containsCPOOL_Parsing) {
            DEF_OPT_ARRAY_HANDLE(CPOOL, cpool, Cpool, cpool, {
                static_assert(!hasCorrectCpool || WithParseSize<decltype(classNode.cpool)>,
                              "Invalid cpool field type (valid requires WithParseSize<TYPE> (from asm.parser_utils, if "
                              "mode != NONE))");

                ParseJaacComplexCollMode mode =
                        shouldHasCorrectCpool
                                ? (parseCpoolStructureFlag ? ParseJaacComplexCollMode::PARSE
                                                           : ParseJaacComplexCollMode::SKIP_AND_SAVE_INTERVALS)
                                : ParseJaacComplexCollMode::FULL_SKIP;

                cp_consts::opt_parse_cpool_coll<mode>(readPtr, cpoolSize, classNode.cpool);
            });

            DEF_EXISTS_GET_AND_FIELD_WRITE(POST_CPOOL, containsCPOOL_Parsing, wrappers::AccessFlags, accessFlags);
            DEF_EXISTS_GET_AND_FIELD_WRITE(POST_CPOOL, containsCPOOL_Parsing, wrappers::CpoolSizeT, thisClass);
            DEF_EXISTS_GET_AND_FIELD_WRITE(POST_CPOOL, containsCPOOL_Parsing, wrappers::CpoolSizeT, superClass);

            if constexpr (containsIFACES_Parsing) {
                DEF_OPT_ARRAY_HANDLE(IFACES, ifaces, Ifaces, ifaces, {
                    if constexpr (shouldHasCorrectIfaces) {
                        for (ArrayRSizeT i = 0; i < ifacesSize; ++i) {
                            PARSE_OR_RET_UNEX(wrappers::CpoolSizeT, iface, IFACES);
                            DEF_ASSERT_UNEX(IFACES, classNode.ifaces.jaac_push_back(iface));
                        }
                    } else {
                        DEF_ASSERT_UNEX(IFACES, readPtr.check_and_move(sizeof(ArrayRSizeT) * ifacesSize));
                    }
                });

                if constexpr (containsFIELDS_Parsing) {
                    DEF_OPT_ARRAY_HANDLE(FIELDS, fields, Fields, fields, {
                        DEF_ASSERT_UNEX(FIELDS,
                                        parse_MF<shouldHasCorrectFields>(readPtr, classNode.fields, fieldsSize));
                    });


                    if constexpr (containsMETHODS_Parsing) {
                        DEF_OPT_ARRAY_HANDLE(METHODS, methods, Methods, methods, {
                            DEF_ASSERT_UNEX(METHODS, parse_MF<shouldHasCorrectMethods>(readPtr, classNode.methods,
                                                                                       methodsSize));
                        });


                        if constexpr (containsFULL_Parsing) {
                            DEF_OPT_ARRAY_HANDLE(ATTRS, attrs, Attrs, attrs, {
                                DEF_ASSERT_UNEX(
                                        ATTRS,
                                        parse_simple_collection<shouldHasCorrectAttrs, common::DefVeryFullAttribute>(
                                                readPtr, classNode.attrs, attrsSize));
                            });
                        }
                    }
                }
            }
        }

        if constexpr (requires {
                          { classNode.postread_handle() } -> gstd::bool_or_true;
                      }) {
            DEF_ASSERT_UNEX(POST_PARSE, classNode.postread_handle());
        }

        auto retL = [&](DeserErr err = {}) -> ParseRetType<ClassNT> {
            if constexpr (IsConvertableAtReturn<ClassNT>) {
                return ParseRetType<ClassNT>{CondPairInParseRetType<ClassNT>{classNode.jaac_return_convert(), err}};
            } else {
                return ParseRetType<ClassNT>{CondPairInParseRetType<ClassNT>{classNode, err}};
            }
        };

        if constexpr (useWriteFlag) {
            // call handlers (deep analyse of version, lol)
            DEF_BOOL_CNT_FLAG_GEN(writeVersion);
            cstd::call_if_no_false(cstd::or_else_falseCPY<writeVersionFlag>(
                    DEF_FIELD_RETURNER(useWriteFlag, wrappers::VersionNumber, version)));
            if constexpr (containsCPOOL_Parsing) {
                DEF_BOOL_CNT_FLAG_GEN(writeAccess);
                DEF_BOOL_CNT_FLAG_GEN(writeThisClassIdx);
                DEF_BOOL_CNT_FLAG_GEN(writeSuperClassIdx);
                COM_BOOL SAC0 = useWriteFlag && containsCPOOL_Parsing;
                cstd::call_if_no_false(cstd::func_or_false_line(
                        cstd::or_else_falseCPY<writeAccessFlag>(DEF_FIELD_RETURNER(SAC0, RawAccessFlagsT, accessFlags)),
                        cstd::or_else_falseCPY<writeThisClassIdxFlag>(DEF_FIELD_RETURNER(SAC0, CpoolRSizeT, thisClass)),
                        cstd::or_else_falseCPY<writeSuperClassIdxFlag>(
                                DEF_FIELD_RETURNER(SAC0, CpoolRSizeT, superClass))));
            }

            // deser
            std::vector<uint8_t>& outData = *outputPtr;
            outData.reserve(inputData.size());
            WritePtr writePtr{outData};

            outData.append_range(opcodes::magic_array_rev);
            version.deser(writePtr);
            if constexpr (containsCPOOL_Parsing) {
                DEF_JAAC_COMPLEX_DESER_SKIP0(CPOOL, Cpool, cpool, wrappers::CpoolSizeT);

                accessFlags.deser(writePtr);
                thisClass.deser(writePtr);
                superClass.deser(writePtr);

                if constexpr (containsIFACES_Parsing) {
                    DEF_JAAC_PRIM_DESER(IFACES, Ifaces, ifaces, wrappers::CpoolSizeT, wrappers::Number<ArrayRSizeT>);
                    if constexpr (containsFIELDS_Parsing) {
                        DEF_JAAC_COMPLEX_DESER(FIELDS, Fields, fields, wrappers::ArraySizeT);
                        if constexpr (containsMETHODS_Parsing) {
                            DEF_JAAC_COMPLEX_DESER(METHODS, Methods, methods, wrappers::ArraySizeT);
                            if constexpr (containsFULL_Parsing) {
                                DEF_JAAC_COMPLEX_DESER(ATTRS, Attrs, attrs, wrappers::ArraySizeT);
                            }
                        }
                    }
                }
            }

            if constexpr (!containsFULL_Parsing) {
                outData.append_range(readPtr);
            }
        }

        return retL();
    }


    template <typename ClassNT /*CNT*/, typename SpecR = cflags::class_file_ptr,
              typename SpecW = cflags::class_file_ptr, bool checkMagic = true>
    ParseRetType<ClassNT> unsafe_parse(std::span<uint8_t> inputData, std::vector<uint8_t>* outputPtr = nullptr) {
        static_assert(JaacUnsafeClassNT<ClassNT, SpecR, SpecW>);

        using ReadPtr = ReadPtrBySpec<SpecR>;
        using WritePtr = WritePtrBySpec<SpecW>;

        ClassNT classNode{};
        DEF_BOOL_CNT_FLAG_GEN(useWrite);
        DEF_CNT_FLAG_GEN(parseLength, ParseLength);
        LENGTH_BOOL_GEN(CPOOL);
        LENGTH_BOOL_GEN(IFACES);
        LENGTH_BOOL_GEN(FIELDS);
        LENGTH_BOOL_GEN(METHODS);
        LENGTH_BOOL_GEN(FULL);

        ReadPtr readPtr{inputData};

        if constexpr (checkMagic) {
            PARSE_OR_RET_UNEX(wrappers::MagicNumber, magic, MAGIC);
            DEF_ASSERT_UNEX(MAGIC, static_cast<uint32_t>(magic) == opcodes::magic);
        } else {
            DEF_ASSERT_UNEX(MAGIC, readPtr.template check_and_move<4>());
        }

        DEF_GET_AND_FIELD_WRITE(true, VERSION, wrappers::VersionNumber, version);

        wrappers::AccessFlags accessFlags;
        wrappers::CpoolSizeT thisClass;
        wrappers::CpoolSizeT superClass;

        if constexpr (containsCPOOL_Parsing) {
            DEF_MANUAL_ARR_HANDLE(CPOOL, cpool);

            DEF_EXISTS_GET_AND_FIELD_WRITE(POST_CPOOL, containsCPOOL_Parsing, wrappers::AccessFlags, accessFlags);
            DEF_EXISTS_GET_AND_FIELD_WRITE(POST_CPOOL, containsCPOOL_Parsing, wrappers::CpoolSizeT, thisClass);
            DEF_EXISTS_GET_AND_FIELD_WRITE(POST_CPOOL, containsCPOOL_Parsing, wrappers::CpoolSizeT, superClass);

            if constexpr (containsIFACES_Parsing) {
                DEF_MANUAL_ARR_HANDLE(IFACES, ifaces);
                if constexpr (containsFIELDS_Parsing) {
                    DEF_MANUAL_ARR_HANDLE(FIELDS, fields);
                    if constexpr (containsMETHODS_Parsing) {
                        DEF_MANUAL_ARR_HANDLE(METHODS, methods);
                        if constexpr (containsFULL_Parsing) {
                            DEF_MANUAL_ARR_HANDLE(ATTRS, attrs);
                        }
                    }
                }
            }
        }

        if constexpr (requires {
                          { classNode.postread_handle() } -> gstd::bool_or_true;
                      }) {
            DEF_ASSERT_UNEX(POST_PARSE, classNode.postread_handle());
        }

        auto retL = [&](DeserErr err = {}) -> ParseRetType<ClassNT> {
            if constexpr (IsConvertableAtReturn<ClassNT>) {
                return ParseRetType<ClassNT>{CondPairInParseRetType<ClassNT>{classNode.jaac_return_convert(), err}};
            } else {
                return ParseRetType<ClassNT>{CondPairInParseRetType<ClassNT>{classNode, err}};
            }
        };

        if constexpr (useWriteFlag) {
            // call handlers (deep analyse of version, lol)
            DEF_BOOL_CNT_FLAG_GEN(writeVersion);
            cstd::call_if_no_false(cstd::or_else_falseCPY<writeVersionFlag>(
                    DEF_FIELD_RETURNER(useWriteFlag, wrappers::VersionNumber, version)));
            if constexpr (containsCPOOL_Parsing) {
                DEF_BOOL_CNT_FLAG_GEN(writeAccess);
                DEF_BOOL_CNT_FLAG_GEN(writeThisClassIdx);
                DEF_BOOL_CNT_FLAG_GEN(writeSuperClassIdx);
                COM_BOOL SAC0 = useWriteFlag && containsCPOOL_Parsing;
                cstd::call_if_no_false(cstd::func_or_false_line(
                        cstd::or_else_falseCPY<writeAccessFlag>(DEF_FIELD_RETURNER(SAC0, RawAccessFlagsT, accessFlags)),
                        cstd::or_else_falseCPY<writeThisClassIdxFlag>(DEF_FIELD_RETURNER(SAC0, CpoolRSizeT, thisClass)),
                        cstd::or_else_falseCPY<writeSuperClassIdxFlag>(
                                DEF_FIELD_RETURNER(SAC0, CpoolRSizeT, superClass))));
            }

            // deser
            std::vector<uint8_t>& outData = *outputPtr;
            outData.reserve(inputData.size());
            WritePtr writePtr{outData};

            outData.append_range(opcodes::magic_array_rev);
            version.deser(writePtr);
            if constexpr (containsCPOOL_Parsing) {
                DEF_ASSERT_EX(CPOOL, classNode.cpool.deser(writePtr));

                accessFlags.deser(writePtr);
                thisClass.deser(writePtr);
                superClass.deser(writePtr);

                if constexpr (containsIFACES_Parsing) {
                    DEF_ASSERT_EX(IFACES, classNode.ifaces.deser(writePtr));
                    if constexpr (containsFIELDS_Parsing) {
                        DEF_ASSERT_EX(FIELDS, classNode.fields.deser(writePtr));
                        if constexpr (containsMETHODS_Parsing) {
                            DEF_ASSERT_EX(METHODS, classNode.methods.deser(writePtr));
                            if constexpr (containsFULL_Parsing) {
                                DEF_ASSERT_EX(ATTRS, classNode.attrs.deser(writePtr));
                            }
                        }
                    }
                }
            }

            if constexpr (!containsFULL_Parsing) {
                outData.append_range(readPtr);
            }
        }

        return retL();
    }
} // namespace parser
