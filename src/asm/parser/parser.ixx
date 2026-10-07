module;

#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <expected>
#include <iostream>
#include <optional>
#include <type_traits>
#include <vector>
#include <span>

#include "parser_mcr.h"

export module parser;

import reader;
import opcodes;
import compl_flags;
import simple_cstd;
import const_i;
import parser_utils;
import parser_concepts;
import array_parsers;
import ptr_function;
import dstd;
import parser_b1;

export namespace parser {
    template <typename ClassNT /*CNT*/, typename SpecR = cflags::class_file_ptr_fs,
              typename SpecW = cflags::class_file_ptr_fs, bool checkMagic = true, bool g1> requires JaacClassNT<ClassNT>
    std::expected<std::pair<ClassNT, ParseResultFlags>, ParseErr>
    parse0(const reader::PreRWFileData<g1> preRWData, const reader::FileDataWithSize<> readData) noexcept {
        using FileReadPtr = FileReadPtrBySpec<SpecR>;
        using FileWritePtr = FileWritePtrBySpec<SpecW>;

        ClassNT classNode{};
        DEF_BOOL_CNT_FLAG_GEN(useWrite);
        DEF_CNT_FLAG_GEN(parseLength, ParseLength);

        FileReadPtr readPtr{readData};

        if constexpr (checkMagic) {
            DEF_OR_ELSE_OPT_UNEX(uint32_t, magic);
            DEF_ASSERT_UNEX(magic == opcodes::magic);
        } else {
            DEF_ASSERT_UNEX(readPtr.template check_and_move<4>());
        }

        DEF_OR_ELSE_OPT_UNEX(uint16_t, minor_version);
        DEF_OR_ELSE_OPT_UNEX(uint16_t, major_version);

        DEF_BOOL_CNT_FLAG_GEN(handleVersion);
        auto versionH = cstd::or_else_falseCPY<handleVersionFlag>(
                cstd::func_or_false_line(DEF_HANDLE(uint16_t, minor_version), DEF_HANDLE(uint16_t, major_version)));

        if constexpr (parseLengthFlag == ParseLength::VERSION) goto write_back_code;

        DEF_OPT_ARRAY_HANDLE(cpool, Cpool, cpool, FileReadPtr, std::span<uint8_t>, {
            static_assert(!hasCorrectCpool || WithParseSize<decltype(classNode.cpool)>,
                          "Invalid cpool field type (valid requires WithParseSize<TYPE> (from asm.parser_utils, if "
                          "mode != NONE))");

            if constexpr (hasCorrectCpool) {
                classNode.cpool.jaac_set_parse_size(cpool_size);
            }

            T_CO_FLAG parseCpoolStructure = handleWrittenCpoolFlag == HandleMode::FULL;

            if constexpr (parseCpoolStructure) {
                classNode.cpool.jaac_push_back(FileReadPtr{nullptr, nullptr});
            }

            for (int i = 1; i < cpool_size; ++i) {
                uint8_t* iOld = readPtr.i;
                parse1_const_big_ctx<false>(i, [](const opcodes::TagIdEnum, cp_consts::traits::variant) {},
                    [&] (const opcodes::TagIdEnum tagId) {
                    classNode.cpool.jaac_push_back(FileReadPtr{iOld, readPtr.i});
                    if (tagId == opcodes::TagIdEnum::CONSTANT_Double || tagId == opcodes::TagIdEnum::CONSTANT_Long) {
                        classNode.cpool.jaac_push_back(FileReadPtr{nullptr, nullptr});
                        i++;
                    }
                });
            }
        });

        DEF_OR_ELSE_OPT_UNEX(uint16_t, access_flags);
        DEF_GET_AND_I_HAND(CPoolSizeT, this_class);
        DEF_OR_ELSE_OPT_UNEX(CPoolSizeT, super_class);

        DEF_BOOL_CNT_FLAG_GEN(handleAccess);
        DEF_BOOL_CNT_FLAG_GEN(deepHandleThisClassIdx);
        DEF_BOOL_CNT_FLAG_GEN(handleSuperClassIdx);

        auto postCPoolSimpleH = cstd::func_or_false_line(
                cstd::or_else_falseCPY<handleAccessFlag>(DEF_HANDLE(AccessFlagsDefT, access_flags)),
                cstd::or_else_falseCPY<deepHandleThisClassIdxFlag>(DEF_HANDLE(CPoolSizeT, this_class)),
                cstd::or_else_falseCPY<handleSuperClassIdxFlag>(DEF_HANDLE(CPoolSizeT, super_class)));

        if constexpr (parseLengthFlag == ParseLength::CPOOL) goto write_back_code;

        DEF_OPT_ARRAY_HANDLE(ifaces, Ifaces, ifaces, ArraySizeT, ArraySizeT, {
            if constexpr (handleWrittenIfacesFlag == HandleMode::FULL) {
                for (int i = 0; i < ifaces_size; ++i) {
                    DEF_OR_ELSE_OPT_UNEX(CPoolSizeT, iface);
                    classNode.ifaces.jaac_push_back(iface);
                }
            } else {
                DEF_CHECK_MEMORY_OR_UNEX(sizeof(CPoolSizeT) * ifaces_size)
            }
        });

        if constexpr (parseLengthFlag == ParseLength::IFACES) goto write_back_code;

        DEF_OPT_ARRAY_HANDLE(fields, Fields, fields, common::FieldInfo<>, {
            for (int i = 0; i < fields_size; ++i) {
                if constexpr (handleWrittenFieldsFlag == HandleMode::FULL) {
                    DEF_OR_ELSE_OPT_UNEX(AccessFlagsDefT, access_flags);
                    classNode.fields.jaac_push_back();
                }
            }
        });

        write_back_code:
        if constexpr (useWriteFlag) {
            // open file
            const auto wDataOpt = preRWData.open("wb");
            if (!wDataOpt)
                return {{classNode, ParseResultFlags::UNWRITE}};
            auto wData = *wDataOpt;

            // call handlers
            versionH();
            DEF_ARRAY_FINAL_H(Cpool, cpool)
            postCPoolSimpleH();
            DEF_ARRAY_FINAL_H(Ifaces, ifaces)

            // adding back
            std::vector<uint8_t> outData;
            outData.reserve(readData.size);
            FileWritePtr writePtr{outData};

            auto backAddingLambda1 = [&outData]<bool swap>(auto handled) {
                auto final_value = swap ? std::byteswap(handled) : handled;
                outData.append_range(cstd::convert_to_span<uint8_t>(final_value));
            };
            auto backAddingLambda = [backAddingLambda1]<bool swap>(auto... handled) {
                (..., backAddingLambda1.template operator()<swap>(handled));
            };
            auto defBackAddingLambda = [backAddingLambda](auto... handled) {
                return backAddingLambda.template operator()<true>(handled);
            };
            auto rawBackAddingLambda = [backAddingLambda](auto... handled) {
                return backAddingLambda.template operator()<false>(handled);
            };

            outData.append_range(opcodes::magic_array_rev);
            defBackAddingLambda(minor_version, major_version);

            ARRAY_SIZE_WR_BACK_WITH_PSIZE(cpool, Cpool, cpool);

            if constexpr (hasCorrectCpool) {
                DEF_SKIP0_JAAC_FOR(cpool, cpool, cp_consts::traits::variant,
                                   {DEF_ASSERT_UNEX(i_cpool.deref_visit([&](auto&& self) {
                                       return writePtr.memcpy_back_rev(opcodes::as_number(self.TAG_ID)) &&
                                              self.back(writePtr);
                                   }))});
            }
            defBackAddingLambda(access_flags);

            DEF_PRIM_FULL_ARRAY_BACK(ifaces, Ifaces, ifaces);

            outData.append_range(readPtr);

            // close file
            wData.raw_write_file(reader::convertToFileDataWS(std::move(outData)));
            wData.close();
        }

        if constexpr (requires { classNode.jaac_return_convert(); }) {
            return {{classNode.jaac_return_convert(), ParseResultFlags::NONE}};
        } else {
            return {{classNode, ParseResultFlags::NONE}};
        }
    }

    template <typename ClassNT, typename SpecR = cflags::class_file_ptr_fs, typename SpecW = cflags::class_file_ptr_fs,
              bool useFlush = true, bool checkMagic = true, bool g1>
    std::expected<std::pair<ClassNT, ParseResultFlags>, ParseErr>
    parse(const reader::PreRWFileData<g1> preRWData) noexcept {
        const auto rDataOpt = preRWData.open("rb");

        if (!rDataOpt)
            return std::unexpected{ParseErr::UNREAD};

        const reader::RWFileData rData = *rDataOpt;

        if constexpr (useFlush) {
            DISCARD rData.flush();
        }

        const auto readData = rData.read_file<uint8_t, true, true>();
        DISCARD rData.close();

        auto ret = parse0<ClassNT, SpecR, SpecW, checkMagic, g1>(preRWData, readData);
        readData.unchecked_close();

        return ret;
    }
} // namespace parser
