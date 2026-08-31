module;

#include <cstdint>
#include <optional>
#include <span>
#include <string>

#include <iostream>
#include "../cstd/macro.h"
#include "const_i_mcr.h"

export module const_i;

import simple_cstd;
import compl_flags;
import opcodes;
import parser_utils;
import dstd;
import cstd_variant;

export namespace const_i {
    using SizeT = uint16_t;

    namespace inst {
        namespace utils {
            template <typename Self, typename... Args, typename Spec>
            NODISCARD std::optional<Self> create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept {
                return std::make_optional(Self{dataPtr.template memcpy_get_rev<Args>()...});
            }
        } // namespace utils

        template <typename PrType>
        struct AbsPrimitive {
            PrType value;

            NODISCARD static std::string to_str() noexcept {
                return std::to_string(static_cast<cstd::GetMinNumStrUType<PrType>>(value));
            }

        protected:
            DEF_ABS_METHODS(AbsPrimitive)
        };

        struct Class {
            SizeT classIdx;

            NODISCARD static std::string to_str() noexcept {
                return "#" + std::to_string(classIdx);
            }

            DEF_METHODS(Class)
        };

        struct AbsMF {
            SizeT classIdx;
            SizeT nameAndTypeIdx;

            NODISCARD static std::string to_str() noexcept {
                return "#" + std::to_string(classIdx) + ".#" + std::to_string(nameAndTypeIdx);
            }

        protected:
            template <typename Spec>
            NODISCARD static std::optional<AbsMF> abs_create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept {
                return utils::create<AbsMF, decltype(classIdx), decltype(nameAndTypeIdx)>(dataPtr);
            }

            DEF_ABS_CHECK_AND_MOVE(AbsMF)
        };

        struct String {
            SizeT stringIdx;

            NODISCARD static std::string to_str() noexcept {
                return "#" + std::to_string(stringIdx);
            }

            DEF_METHODS(String)
        };

        struct AbsMP {
            SizeT nameIdx;

            NODISCARD static std::string to_str() noexcept {
                return "#" + std::to_string(nameIdx);
            }

        protected:
            DEF_ABS_METHODS(AbsMP)
        };

        struct MethodHandle {
            uint16_t refIdx;
            uint8_t refKind;

            NODISCARD static std::string to_str() noexcept {
                return "#" + std::to_string(nameIdx);
            }

            template <typename Spec>
            NODISCARD static constexpr std::optional<MethodHandle>
            create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept {
                const auto refKind = dataPtr.template memcpy_get_rev<uint8_t>();
                const auto refIdx = dataPtr.template memcpy_get_rev<uint16_t>();
                if (!refIdx)
                    return std::nullopt;
                return {MethodHandle{*refIdx, *refKind}};
            }

            DEF_CHECK_AND_MOVE(MethodHandle)
        };

        struct NameAndType {
            SizeT nameIdx, descIdx;

            NODISCARD static std::string to_str() noexcept {
                return "#" + std::to_string(nameIdx) + ":#" + std::to_string(descIdx);
            }

            template <typename Spec>
            NODISCARD static std::optional<NameAndType> create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept {
                return utils::create<NameAndType, decltype(nameIdx), decltype(descIdx)>(dataPtr);
            }

            DEF_CHECK_AND_MOVE(NameAndType)
        };

        struct MethodType {
            SizeT descIdx;

            NODISCARD static std::string to_str() noexcept {
                return "#" std::to_string(descIdx);
            }

            DEF_METHODS(MethodType)
        };

        struct AbsDynamic {
            SizeT bootstrapMethodAttrIdx, nameAndTypeIdx;

            template <typename Spec>
            NODISCARD static std::optional<AbsDynamic> create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept {
                return utils::create<AbsDynamic, decltype(bootstrapMethodAttrIdx), decltype(nameAndTypeIdx)>(dataPtr);
            }

            DEF_CHECK_AND_MOVE(AbsDynamic)
        };

        struct Utf8 {
            using Utf8SizeT = uint16_t;

            std::span<uint8_t> utf8Info;
            bool shouldFree;

            template <typename Spec>
            NODISCARD static constexpr std::optional<Utf8> create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept {
                const auto lengthOpt =
                        dataPtr.template memcpy_get_rev<Utf8SizeT>().transform(cstd::cast_lambda<size_t>);
                if (!lengthOpt)
                    return std::nullopt;

                uint8_t* strStartPtr = dataPtr.getI();
                if (const size_t length = *lengthOpt; dataPtr.check_memory_and_move(length)) {
                    return {Utf8{{strStartPtr, length}, false}};
                }
                return std::nullopt;
            }

            template <typename Spec>
            NODISCARD static constexpr bool check_and_move(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept {
                const auto lengthOpt =
                        dataPtr.template memcpy_get_rev<Utf8SizeT>().transform(cstd::cast_lambda<size_t>);
                if (!lengthOpt)
                    return false;
                const size_t length = *lengthOpt;
                return dataPtr.check_memory_and_move(length);
            }
        };

        namespace spec {
            DEF_ABS_EXT_STRUCT(PrInt, AbsPrimitive<opcodes::java_ts::Int>, CONSTANT_Integer)
            DEF_ABS_EXT_STRUCT(PrFloat, AbsPrimitive<opcodes::java_ts::Float>, CONSTANT_Float)
            DEF_ABS_EXT_STRUCT(PrDouble, AbsPrimitive<opcodes::java_ts::Double>, CONSTANT_Double)
            DEF_ABS_EXT_STRUCT(PrLong, AbsPrimitive<opcodes::java_ts::Long>, CONSTANT_Long)
            DEF_ABS_EXT_STRUCT(FRef, AbsMF, CONSTANT_Fieldref)
            DEF_ABS_EXT_STRUCT(MRef, AbsMF, CONSTANT_Methodref)
            DEF_ABS_EXT_STRUCT(IMRef, AbsMF, CONSTANT_InterfaceMethodref)
            DEF_ABS_EXT_STRUCT(Module, AbsMP, CONSTANT_Module)
            DEF_ABS_EXT_STRUCT(Package, AbsMP, CONSTANT_Package)
            DEF_ABS_EXT_STRUCT(Dynamic, AbsDynamic, CONSTANT_Dynamic)
            DEF_ABS_EXT_STRUCT(IDynamic, AbsDynamic, CONSTANT_InvokeDynamic)
        } // namespace spec
    } // namespace inst

    namespace traits {
        using Integer = inst::spec::PrInt;
        using Float = inst::spec::PrFloat;
        using Long = inst::spec::PrLong;
        using Double = inst::spec::PrDouble;
        using Class = inst::Class;
        using String = inst::String;
        using MethodHandle = inst::MethodHandle;
        using MethodType = inst::MethodType;
        using Dynamic = inst::AbsDynamic;
        using Utf8 = inst::Utf8;
        using Fieldref = inst::spec::FRef;
        using Methodref = inst::spec::MRef;
        using InterfaceMethodref = inst::spec::IMRef;
        using NameAndType = inst::NameAndType;
        using InvokeDynamic = inst::AbsDynamic;
        using Module = inst::spec::Module;
        using Package = inst::spec::Package;

        using All = dstd::args::TypesTraits<Integer, Float, Long, Double, Class, String, MethodHandle, MethodType,
                                            Dynamic, Utf8, Fieldref, Methodref, InterfaceMethodref, NameAndType,
                                            InvokeDynamic, Module, Package>;
        using variant = All::Wrap<cstd::variant_of_ptrs_and_little>;

        std::string variant_to_str(const variant var) noexcept {
            return var.deref_visit([] <typename Type>(Type&& value) -> std::string {
                return value.to_str();
            })
        }
    } // namespace traits

    template <typename LambdaRetType>
    auto swith_tag_id_enum_2compl(const opcodes::TagIdEnum tagId, auto&& callable) noexcept {
        switch (tagId) {
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Integer, Integer);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Float, Float);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Long, Long);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Double, Double);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Class, Class);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_String, String);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_MethodHandle, MethodHandle);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_MethodType, MethodType);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Dynamic, Dynamic);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Utf8, Utf8);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Fieldref, Fieldref);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Methodref, Methodref);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_InterfaceMethodref, InterfaceMethodref);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_NameAndType, NameAndType);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_InvokeDynamic, InvokeDynamic);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Module, Module);
            SWITCH_TAG_CASE_GEN_2COMPL(opcodes::TagIdEnum::CONSTANT_Package, Package);
            [[unlikely]] default:
                return std::optional<LambdaRetType>{std::nullopt};
        }
    }

    template <typename LambdaRetType>
    auto swith_tag_id_enum_1compl_1rt(const opcodes::TagIdEnum tagId, auto&& callable) noexcept {
        switch (tagId) {
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Integer, Integer);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Float, Float);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Long, Long);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Double, Double);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Class, Class);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_String, String);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_MethodHandle, MethodHandle);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_MethodType, MethodType);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Dynamic, Dynamic);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Utf8, Utf8);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Fieldref, Fieldref);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Methodref, Methodref);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_InterfaceMethodref, InterfaceMethodref);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_NameAndType, NameAndType);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_InvokeDynamic, InvokeDynamic);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Module, Module);
            SWITCH_TAG_CASE_GEN_1COMPL_1RT(opcodes::TagIdEnum::CONSTANT_Package, Package);
            [[unlikely]] default:
                return std::optional<LambdaRetType>{std::nullopt};
        }
    }

    template <typename LambdaRetType>
    auto swith_tag_id_enum_1compl(const opcodes::TagIdEnum tagId, auto&& callable) noexcept {
        switch (tagId) {
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Integer);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Float);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Long);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Double);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Class);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_String);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_MethodHandle);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_MethodType);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Dynamic);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Utf8);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Fieldref);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Methodref);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_InterfaceMethodref);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_NameAndType);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_InvokeDynamic);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Module);
            SWITCH_TAG_CASE_GEN_1COMPL(opcodes::TagIdEnum::CONSTANT_Package);
            [[unlikely]] default:
                return std::optional<LambdaRetType>{std::nullopt};
        }
    }

    constexpr std::optional<std::string_view> tag_to_str_view(const opcodes::TagIdEnum tagId) noexcept {
        switch (tagId) {
            SWITCH_TAG_CASE_GEN_TO_STR_V(Integer);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Float);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Long);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Double);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Class);
            SWITCH_TAG_CASE_GEN_TO_STR_V(String);
            SWITCH_TAG_CASE_GEN_TO_STR_V(MethodHandle);
            SWITCH_TAG_CASE_GEN_TO_STR_V(MethodType);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Dynamic);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Utf8);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Fieldref);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Methodref);
            SWITCH_TAG_CASE_GEN_TO_STR_V(InterfaceMethodref);
            SWITCH_TAG_CASE_GEN_TO_STR_V(NameAndType);
            SWITCH_TAG_CASE_GEN_TO_STR_V(InvokeDynamic);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Module);
            SWITCH_TAG_CASE_GEN_TO_STR_V(Package);
            [[unlikely]] default:
                return std::nullopt;
        }
    }

    std::string tag_to_str(const opcodes::TagIdEnum tagId) noexcept {
        const auto name = tag_to_str_view(tagId);
        return name ? static_cast<std::string>(*name) : "NA: " + std::to_string(static_cast<opcodes::TagIdType>(tagId));
    }

    std::ostream& operator<<(std::ostream& stream, const opcodes::TagIdEnum tagId) {
        return stream << tag_to_str(tagId);
    }
} // namespace const_i
