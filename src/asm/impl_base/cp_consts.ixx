module;

#include <cstdint>
#include <optional>
#include <span>
#include <string>

#include <iostream>
#include "../../cstd/macro.h"
#include "cp_consts_mcr.h"

export module const_i;

import simple_cstd;
import compl_flags;
import opcodes;
import parser_utils;
import dstd;
import cstd_variant;
import read_utils;

export namespace cp_consts {
    using SizeT = parser::CPoolSizeT;
    namespace inst {
        template <typename PrType>
        struct AbsPrimitive {
            PrType value;

            NODISCARD std::string to_str() const noexcept {
                return std::to_string(static_cast<cstd::GetMinNumStrUType<PrType>>(value));
            }

            DEF_ABS_METHODS_PACK(AbsPrimitive, value)
        };

        struct Class {
            SizeT classIdx;
            DEF_TAG_FLAG(CONSTANT_Class)

            NODISCARD std::string to_str() const noexcept {
                return "#" + std::to_string(classIdx);
            }

            DEF_METHODS(Class, classIdx)
        };

        struct AbsMF {
            SizeT classIdx;
            SizeT nameAndTypeIdx;

            NODISCARD std::string to_str() const noexcept {
                return "#" + std::to_string(classIdx) + ".#" + std::to_string(nameAndTypeIdx);
            }

            DEF_DESER(classIdx, nameAndTypeIdx)

        protected:
            template <typename SpecR>
            NODISCARD static std::optional<AbsMF> abs_parse(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
                return reader::utils::parse_rev<AbsMF, decltype(classIdx), decltype(nameAndTypeIdx)>(dataPtr);
            }

            DEF_ABS_CHECK_AND_MOVE(AbsMF)
        };

        struct String {
            SizeT stringIdx;
            DEF_TAG_FLAG(CONSTANT_String)

            NODISCARD std::string to_str() const noexcept {
                return "#" + std::to_string(stringIdx);
            }

            DEF_METHODS(String, stringIdx)
        };

        struct AbsMP {
            SizeT nameIdx;

            NODISCARD std::string to_str() const noexcept {
                return "#" + std::to_string(nameIdx);
            }

            DEF_ABS_METHODS_PACK(AbsMP, nameIdx)
        };

        struct MethodHandle {
            uint16_t refIdx;
            uint8_t refKind;
            DEF_TAG_FLAG(CONSTANT_MethodHandle)

            NODISCARD std::string to_str() const noexcept {
                return std::to_string(refKind) + ":#" + std::to_string(refIdx);
            }

            template <typename SpecR>
            NODISCARD static constexpr std::optional<MethodHandle> parse(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
                const auto refKind = dataPtr.template parse_rev<uint8_t>();
                const auto refIdx = dataPtr.template parse_rev<uint16_t>();
                if (!(refKind && refIdx)) [[unlikely]]
                    return std::nullopt;
                return {MethodHandle{.refIdx = *refIdx, .refKind = *refKind}};
            }

            template <typename SpecR>
            NODISCARD static constexpr bool check_and_move(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
                return reader::utils::check_and_move<decltype(refIdx), decltype(refKind)>(dataPtr);;
            }

            DEF_DESER(refKind, refIdx)
        };

        struct NameAndType {
            SizeT nameIdx, descIdx;
            DEF_TAG_FLAG(CONSTANT_NameAndType)

            NODISCARD std::string to_str() const noexcept {
                return "#" + std::to_string(nameIdx) + ":#" + std::to_string(descIdx);
            }

            template <typename SpecR>
            NODISCARD static std::optional<NameAndType> parse(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
                return reader::utils::parse_rev<NameAndType, decltype(nameIdx), decltype(descIdx)>(dataPtr);
            }

            DEF_DESER(nameIdx, descIdx)
            DEF_CHECK_AND_MOVE(NameAndType)
        };

        struct MethodType {
            SizeT descIdx;
            DEF_TAG_FLAG(CONSTANT_MethodType)

            NODISCARD std::string to_str() const noexcept {
                return "#" + std::to_string(descIdx);
            }

            DEF_METHODS(MethodType, descIdx)
        };

        struct AbsDynamic {
            SizeT bootstrapMethodAttrIdx, nameAndTypeIdx;

            NODISCARD std::string to_str() const noexcept {
                return "#" + std::to_string(bootstrapMethodAttrIdx) + ":#" + std::to_string(nameAndTypeIdx);
            }

            DEF_DESER(bootstrapMethodAttrIdx, nameAndTypeIdx)

        protected:
            template <typename SpecR>
            NODISCARD static std::optional<AbsDynamic> abs_parse(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
                return reader::utils::parse_rev<AbsDynamic, decltype(bootstrapMethodAttrIdx), decltype(nameAndTypeIdx)>(dataPtr);
            }

            DEF_ABS_CHECK_AND_MOVE(AbsDynamic)
        };

        template <bool shouldFree = false>
        struct Utf8 {
            std::span<uint8_t> utf8Info;
            using Utf8SizeT = uint16_t;
            DEF_TAG_FLAG(CONSTANT_Utf8)

            NODISCARD std::string to_str() const noexcept {
                return {utf8Info.begin(), utf8Info.end()};
            }

            template <typename SpecR>
            NODISCARD static constexpr std::optional<Utf8> parse(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
                const auto lengthOpt =
                        dataPtr.template parse_rev<Utf8SizeT>().transform(cstd::cast_lambda<size_t>);
                if (!lengthOpt) [[unlikely]]
                    return std::nullopt;

                uint8_t* strStartPtr = dataPtr.i;
                if (const size_t length = *lengthOpt; dataPtr.check_and_move(length)) [[likely]] {
                    return std::make_optional(Utf8(std::span{strStartPtr, length}));
                    // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
                } else {
                    return std::nullopt;
                }
            }

            template <typename SpecR>
            NODISCARD static constexpr bool check_and_move(parser::FileReadPtrBySpec<SpecR>& dataPtr) noexcept {
                const auto lengthOpt =
                        dataPtr.template parse_rev<Utf8SizeT>().transform(cstd::cast_lambda<size_t>);
                if (!lengthOpt) [[unlikely]]
                    return false;
                const size_t length = *lengthOpt;
                return dataPtr.check_and_move(length);
            }

            NODISCARD bool deser(parser::FileWritePtrBySpec<auto>& dataPtr) const noexcept {
                if (!dataPtr.deser_rev(static_cast<Utf8SizeT>(utf8Info.size()))) [[unlikely]] return false;
                for (auto i : utf8Info) if (!dataPtr.deser(i)) [[unlikely]] return false;
                return true;
            }

            ~Utf8() noexcept {
                if constexpr (shouldFree) {
                    delete[] utf8Info.data();
                }
            }

            /*implicit*/ F_INLINE Utf8(const std::span<uint8_t> utf8Info) : utf8Info(utf8Info) {
            }

            explicit Utf8(Utf8&& old) noexcept : Utf8(old.utf8Info) {
                old.utf8Info = {};
            }

            explicit Utf8(Utf8& old) noexcept : Utf8(old.utf8Info) {
                old.utf8Info = {};
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
        using Dynamic = inst::spec::Dynamic;
        using Utf8 = inst::Utf8<>;
        using Fieldref = inst::spec::FRef;
        using Methodref = inst::spec::MRef;
        using InterfaceMethodref = inst::spec::IMRef;
        using NameAndType = inst::NameAndType;
        using InvokeDynamic = inst::spec::IDynamic;
        using Module = inst::spec::Module;
        using Package = inst::spec::Package;

        using Utf8Custom = inst::Utf8<true>;

        struct EmptyVarValue {};

        using All = dstd::args::TypesTraits<Integer, Float, Long, Double, Class, String, MethodHandle, MethodType,
                                            Dynamic, Utf8, Fieldref, Methodref, InterfaceMethodref, NameAndType,
                                            InvokeDynamic, Module, Package>;

        using VariantAllUnempt = All::Append<Utf8Custom>;
        using VariantAll = VariantAllUnempt::Prepend<EmptyVarValue>;

        using variant = VariantAll::Wrap<cstd::emp_ignore_variant_of_ptrs_and_little_t_ptr>;

        using opt_variant = cstd::var_opt<variant>;

        // ReSharper disable once CppPassValueParameterByConstReference
        std::string variant_to_str(const variant var) noexcept { // NOLINT(*-unnecessary-value-param)
            return var.deref_visit([]<typename Type>(Type&& value) -> std::string { return value.to_str(); });
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
