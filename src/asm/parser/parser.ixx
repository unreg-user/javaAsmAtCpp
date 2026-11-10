module;

#include <cassert>
#include <concepts>
#include <cstdint>
#include <expected>
#include <iostream>
#include <optional>
#include <span>
#include <type_traits>
#include <vector>

#include "parser_mcr.h"

export module parser;

import reader;
import opcodes;
import compl_flags;
import simple_cstd;
import gstd;
import cp_consts;
import parser_utils;
import parser_concepts;
import array_parsers;
import ptr_function;
import dstd;
import parser_b1;
import parser_b2;
import parser_wrappers_b2;
import costd;

/**
 * According to the specification, if a collection in classNode is a JaacTranslator, the invocation of its
 * (JaacTranslator) methods is guaranteed; if it is a JaacCollection, the invocation of its (JaacCollection) methods is
 * not guaranteed.
 *
 */
export namespace parser {
    template <typename ClassNT>
    using CondPairInParseRetType =
            gstd::cond_pair<ClassNT, std::conditional_t<useWriteConcept<ClassNT>, DeserErr,
                                                        gstd::ccompl_value<DeserErr, DeserErr::NONE>>>;

    template <typename ClassNT>
    using ParseRetType = std::expected<CondPairInParseRetType<JaacClassNTRetConverted<ClassNT>>, ParseErr>;

    template <typename ClassNT /*CNT*/, typename SpecR = cflags::class_file_ptr,
              typename SpecW = cflags::class_file_ptr, bool checkMagic = true, bool g1>
        requires JaacClassNT<ClassNT>
    ParseRetType<ClassNT> parse1(const reader::PreRWFileData<g1> preRWData,
                                 const reader::FileDataWithSize<> readData) noexcept {
        using ReadPtr = ReadPtrBySpec<SpecR>;
        using WritePtr = WritePtrBySpec<SpecW>;

        ClassNT classNode{};
        DEF_BOOL_CNT_FLAG_GEN(useWrite);
        DEF_CNT_FLAG_GEN(parseLength, ParseLength);

        ReadPtr readPtr{readData};

        if constexpr (checkMagic) {
            DEF_OR_ELSE_OPT_UNEX(wrappers::MagicNumber, magic);
            DEF_ASSERT_UNEX(static_cast<uint32_t>(magic) == opcodes::magic);
        } else {
            DEF_ASSERT_UNEX(readPtr.template check_and_move<4>());
        }

        DEF_OR_ELSE_OPT_UNEX(wrappers::VersionNumber, version);

        DEF_BOOL_CNT_FLAG_GEN(handleVersion);
        // deep analys of version, lol
        auto versionH = cstd::or_else_falseCPY<handleVersionFlag>(DEF_HANDLE(wrappers::VersionNumber, version));

        DEF_BOOL_CNT_FLAG_GEN(parseCpoolStructure);
        using CpoolArrInputT = std::conditional_t<parseCpoolStructureFlag, cp_consts::traits::cp_var, u8_span>;

        DEF_COM_ARRAY_HANDLE(Cpool, cpool, CpoolArrInputT);
        DEF_COM_ARRAY_HANDLE(Ifaces, ifaces, wrappers::CpoolSizeT);
        DEF_COM_ARRAY_HANDLE(Fields, fields, common::MFInfo<>);
        DEF_COM_ARRAY_HANDLE(Methods, methods, common::MFInfo<>);
        DEF_COM_ARRAY_HANDLE(Attrs, attrs, common::DefVeryFullAttribute);

        wrappers::AccessFlags access_flags;
        wrappers::CpoolSizeT this_class;
        wrappers::CpoolSizeT super_class;

        DEF_BOOL_CNT_FLAG_GEN(handleAccess);
        DEF_BOOL_CNT_FLAG_GEN(deepHandleThisClassIdx);
        DEF_BOOL_CNT_FLAG_GEN(handleSuperClassIdx);

        auto postCPoolSimpleHRetsLambda = [&] {
            return cstd::func_or_false_line(
                    cstd::or_else_falseCPY<handleAccessFlag>(DEF_HANDLE(RawAccessFlagsT, access_flags)),
                    cstd::or_else_falseCPY<deepHandleThisClassIdxFlag>(DEF_HANDLE(CpoolRSizeT, this_class)),
                    cstd::or_else_falseCPY<handleSuperClassIdxFlag>(DEF_HANDLE(CpoolRSizeT, super_class)));
        };

        std::conditional_t<parseLengthFlag >= ParseLength::CPOOL, decltype((postCPoolSimpleHRetsLambda())), bool>
                postCPoolSimpleH;

        if constexpr (parseLengthFlag >= ParseLength::CPOOL) {
            DEF_OPT_ARRAY_HANDLE(cpool, Cpool, cpool, {
                static_assert(!hasCorrectCpool || WithParseSize<decltype(classNode.cpool)>,
                              "Invalid cpool field type (valid requires WithParseSize<TYPE> (from asm.parser_utils, if "
                              "mode != NONE))");

                if constexpr (shouldHasCorrectCpool) {
                    classNode.cpool.jaac_set_parse_size(cpool_size);

                    if constexpr (parseCpoolStructureFlag) {
                        DEF_ASSERT_UNEX(classNode.cpool.jaac_push_back(cp_consts::traits::cp_var{}));
                    } else {
                        DEF_ASSERT_UNEX(classNode.cpool.jaac_push_back(u8_span{}));
                    }
                }

                for (ArrayRSizeT i = 1; i < static_cast<ArrayRSizeT>(cpool_size); ++i) {
                    u8_ptr iOld = readPtr.i;
                    bool is_unex = false;
                    auto skipLDIdx = [&](const opcodes::TagIdEnum tagId) {
                        if (tagId == opcodes::TagIdEnum::CONSTANT_Double ||
                            tagId == opcodes::TagIdEnum::CONSTANT_Long) {
                            if constexpr (shouldHasCorrectCpool) {
                                if constexpr (parseCpoolStructureFlag) {
                                    is_unex = is_unex || !classNode.cpool.jaac_push_back(cp_consts::traits::cp_var{});
                                } else {
                                    is_unex = is_unex || !classNode.cpool.jaac_push_back(u8_span{});
                                }
                            }
                            i++;
                        }
                    };
                    cp_consts::parse1_const_big_ctx<parseCpoolStructureFlag>(
                            readPtr,
                            cstd::overload{[&](const opcodes::TagIdEnum tagId, cp_consts::traits::cp_var var) {
                                               if constexpr (parseCpoolStructureFlag && hasCorrectCpool)
                                                   is_unex = !classNode.cpool.jaac_push_back(var);
                                               skipLDIdx(tagId);
                                           },
                                           [&](const opcodes::TagIdEnum tagId) {
                                               if constexpr (!parseCpoolStructureFlag && hasCorrectCpool)
                                                   is_unex = !classNode.cpool.jaac_push_back(
                                                           static_cast<u8_span>(UncheckedReadPtr{iOld, readPtr.i - 1}));
                                               skipLDIdx(tagId);
                                           }});
                    DEF_ASSERT_UNEX(!is_unex)
                }
            });

            {
                auto timed = wrappers::AccessFlags::parse(readPtr);
                if (!timed) [[unlikely]] {
                    return std::unexpected{ParseErr::INVALID};
                }
                access_flags = *timed;
            }
            {
                auto timed = wrappers::CpoolSizeT::parse(readPtr);
                if (!timed) [[unlikely]] {
                    return std::unexpected{ParseErr::INVALID};
                }
                this_class = *timed;
            }
            DEF_I_HANDLE(wrappers::CpoolSizeT, this_class);
            {
                auto timed = wrappers::CpoolSizeT::parse(readPtr);
                if (!timed) [[unlikely]] {
                    return std::unexpected{ParseErr::INVALID};
                }
                super_class = *timed;
            }

            postCPoolSimpleH = postCPoolSimpleHRetsLambda();

            if constexpr (parseLengthFlag >= ParseLength::IFACES) {
                DEF_OPT_ARRAY_HANDLE(ifaces, Ifaces, ifaces, {
                    if constexpr (shouldHasCorrectIfaces) {
                        for (ArrayRSizeT i = 0; i < static_cast<ArrayRSizeT>(ifaces_size); ++i) {
                            DEF_OR_ELSE_OPT_UNEX(wrappers::CpoolSizeT, iface);
                            DEF_ASSERT_UNEX(classNode.ifaces.jaac_push_back(iface));
                        }
                    } else {
                        DEF_CHECK_MEMORY_OR_UNEX(sizeof(ArrayRSizeT) * static_cast<ArrayRSizeT>(ifaces_size))
                    }
                });

                if constexpr (parseLengthFlag >= ParseLength::FIELDS) {
                    DEF_OPT_ARRAY_HANDLE(fields, Fields, fields,
                                         { parse_MF<shouldHasCorrectFields>(readPtr, classNode.fields, fields_size); });


                    if constexpr (parseLengthFlag >= ParseLength::METHODS) {
                        DEF_OPT_ARRAY_HANDLE(methods, Methods, methods, {
                            parse_MF<shouldHasCorrectMethods>(readPtr, classNode.methods, methods_size);
                        });


                        if constexpr (parseLengthFlag >= ParseLength::FULL) {
                            DEF_OPT_ARRAY_HANDLE(attrs, Attrs, attrs, {
                                parse_simple_collection<shouldHasCorrectAttrs, common::DefVeryFullAttribute>(
                                        readPtr, classNode.attrs, attrs_size);
                            });
                        }
                    }
                }
            }
        }

        if constexpr (requires { classNode.postread_handle(); })
            classNode.postread_handle();

        auto retValLambda = [&](DeserErr flag = DeserErr::NONE) -> ParseRetType<ClassNT> {
            if constexpr (requires { classNode.jaac_return_convert(); }) {
                return ParseRetType<ClassNT>{CondPairInParseRetType<ClassNT>{classNode.jaac_return_convert(), flag}};
            } else {
                return ParseRetType<ClassNT>{CondPairInParseRetType<ClassNT>{classNode, flag}};
            }
        };

        if constexpr (useWriteFlag) {
            // open file
            const auto wDataOpt = preRWData.open("wb");
            if (!wDataOpt)
                return retValLambda(DeserErr::UNOPEN);
            auto wData = *wDataOpt;

            // call handlers
            versionH();
            if constexpr (parseLengthFlag >= ParseLength::CPOOL) {
                DEF_ARRAY_PREWRITE_CALL(Cpool, cpool)
                postCPoolSimpleH();
                if constexpr (parseLengthFlag >= ParseLength::IFACES) {
                    DEF_ARRAY_PREWRITE_CALL(Ifaces, ifaces)
                    if constexpr (parseLengthFlag >= ParseLength::FIELDS) {
                        DEF_ARRAY_PREWRITE_CALL(Fields, fields)
                        if constexpr (parseLengthFlag >= ParseLength::METHODS) {
                            DEF_ARRAY_PREWRITE_CALL(Methods, methods)
                            if constexpr (parseLengthFlag >= ParseLength::FULL) {
                                DEF_ARRAY_PREWRITE_CALL(Attrs, attrs)
                            }
                        }
                    }
                }
            }

            // deser
            std::vector<uint8_t> outData;
            outData.reserve(readData.size);
            WritePtr writePtr{outData};

            outData.append_range(opcodes::magic_array_rev);
            version.deser(writePtr);

            if constexpr (parseLengthFlag >= ParseLength::CPOOL) {
                DEF_JAAC_COMPLEX_DESER(Cpool, cpool);

                access_flags.deser(writePtr);
                this_class.deser(writePtr);
                super_class.deser(writePtr);

                DEF_JAAC_PRIM_DESER(Ifaces, ifaces, wrappers::CpoolSizeT);
                DEF_JAAC_COMPLEX_DESER(Fields, fields);
                DEF_JAAC_COMPLEX_DESER(Methods, methods);
                DEF_JAAC_COMPLEX_DESER(Attrs, attrs);
            }

            outData.append_range(readPtr);

            // close file
            wData.raw_write_file(reader::convertToFileDataWS(std::move(outData)));
            wData.close();
        }

        return retValLambda();
    }

    template <typename ClassNT, typename SpecR = cflags::class_file_ptr, typename SpecW = cflags::class_file_ptr,
              bool useFlush = true, bool checkMagic = true, bool g1>
        requires JaacClassNT<ClassNT>
    ParseRetType<ClassNT> parse(const reader::PreRWFileData<g1> preRWData) noexcept {
        const auto rDataOpt = preRWData.open("rb");

        if (!rDataOpt)
            return std::unexpected{ParseErr::UNOPEN};

        const reader::RWFileData rData = *rDataOpt;

        if constexpr (useFlush) {
            DISCARD rData.flush();
        }

        const auto readData = rData.read_file<uint8_t, true, true>();
        DISCARD rData.close();

        auto ret = parse1<ClassNT, SpecR, SpecW, checkMagic, g1>(preRWData, readData);
        readData.unchecked_close();

        return ret;
    }
} // namespace parser
