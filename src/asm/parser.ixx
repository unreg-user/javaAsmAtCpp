module;

#include <bit>
#include <concepts>
#include <cstdint>
#include <expected>
#include <iostream>
#include <optional>
#include <vector>
#include <cassert>
#include <string>

#include "../cstd/macro.h"
#include "parser_mcr.h"

export module parser;

import reader;
import opcodes;
import compl_flags;
import simple_cstd;
import const_i;
import parser_utils;

namespace parser {
    export {
        enum class ParseErr : uint8_t { UNREAD, INVALID };

        enum class ParseResultFlags : uint8_t { NONE, UNWRITE };
    }

    template <typename ClassNT /*CNT*/, typename Spec = cflags::class_file_ptr_fs, bool checkMagic = true, bool g1>
    std::expected<std::pair<ClassNT, ParseResultFlags>, ParseErr>
    parse0(const reader::PreRWFileData<g1> preRWData, const reader::FileDataWithSize<> readData) noexcept {
        using FileSymPtr = FileSymPtrBySpec<Spec>;
        DEF_STATIC_BOOL_CNT_FLAG_GEN(useWrite, false);
        ClassNT classNode{};

        FileSymPtr symPtr{readData};

        std::vector<uint8_t> outData;

        auto defBackAddingLambda1 = [&outData](auto handled) {
            auto timed = std::byteswap(handled);
            outData.append_range(cstd::convert_to_span<uint8_t>(timed));
        };

        auto defBackAddingLambda = [defBackAddingLambda1](auto... handled) {
            (defBackAddingLambda1(handled), ...);
        };

        if constexpr (useWriteFlag) {
            outData = std::vector<uint8_t>{};
            outData.reserve(readData.size);
            outData.append_range(opcodes::magic_array);
        }

        if constexpr (checkMagic) {
            DEF_OR_ELSE_OPT_UNEX(uint32_t, magic);
            DEF_ASSERT_UNEX(magic == opcodes::magic);
        } else {
            DEF_ASSERT_UNEX(symPtr.template check_memory_and_move<4>());
        }

        DEF_OR_ELSE_OPT_UNEX(uint16_t, minor_version);
        DEF_OR_ELSE_OPT_UNEX(uint16_t, major_version);


        if constexpr (DEF_STATIC_BOOL_CNT_FLAG_RET(handleVersion, false)) {
            DEF_HANDLE_MU_LAMBDA(version, minor_version, major_version)([&] {
                DEF_HANDLE(uint16_t, minor_version);
                DEF_HANDLE(uint16_t, major_version);
            });
        }

        if constexpr (useWriteFlag) {
            defBackAddingLambda(minor_version, major_version);
        }

        DEF_OR_ELSE_OPT_UNEX(uint16_t, cpool_size);
        DEF_STATIC_BOOL_CNT_FLAG_GEN(handleWritedCpool, false);

        {
            const auto handle_lambda = [&symPtr] {
                if constexpr (handleWritedCpoolFlag) {
                    return [&symPtr]<opcodes::TagIdEnum, typename Trait>() noexcept -> const_i::traits::variant {
                        return {const_i::ConstInfo<Trait>::create(symPtr)};
                    };
                } else {
                    return [&symPtr]<opcodes::TagIdEnum, typename Trait>() noexcept -> bool {
                        return const_i::ConstInfo<Trait>::check_and_move(symPtr);
                    };
                };
            }();

            for (int i = 1; i < cpool_size; ++i) {
                DEF_OR_ELSE_OPT_UNEX(opcodes::TagIdType, tagIdRaw);
                auto tagId = static_cast<opcodes::TagIdEnum>(tagIdRaw);

                auto result = const_i::swith_tag_id_enum_2compl<std::conditional_t<handleWritedCpoolFlag, const_i::traits::variant, bool>>(tagId, handle_lambda);
                if constexpr (handleWritedCpoolFlag) {
                    //NOTE: write.
                } else {
                    std::cout << "#" << std::to_string(i) << " = " << const_i::tag_to_str(tagId) << '\n';
                    DEF_ASSERT_UNEX(result);
                    DEF_ASSERT_UNEX(*result);
                }

                if (tagId == opcodes::TagIdEnum::CONSTANT_Double || tagId == opcodes::TagIdEnum::CONSTANT_Long) {
                    i++;
                }
            }
        }

        if constexpr (useWriteFlag) {
            const auto wDataOpt = preRWData.open("wb");
            if (!wDataOpt)
                return {{classNode, ParseResultFlags::UNWRITE}};
            auto wData = *wDataOpt;
            wData.raw_write_file(reader::convertToFileDataWS(std::move(outData)));
            wData.close();
        }

        return {{classNode, ParseResultFlags::NONE}};
    }

    export {
        template <typename ClassNT, typename Spec = cflags::class_file_ptr_fs, bool useFlush = false, bool checkMagic = true, bool g1>
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

            auto ret = parse0<ClassNT, Spec, checkMagic, g1>(preRWData, readData);
            readData.unchecked_close();

            return ret;
        }
    }
} // namespace parser
