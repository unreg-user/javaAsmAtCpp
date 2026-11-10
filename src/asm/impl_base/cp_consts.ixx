module;

#include <cstdint>
#include <optional>
#include <span>
#include <string>

#include <iostream>
#include "../../cstd/macro.h"
#include "../parser/parser_mcr.h"
#include "cp_consts_mcr.h"

export module cp_consts;

import simple_cstd;
import compl_flags;
import opcodes;
import parser_utils;
import dstd;
import cstd_variant;
import read_utils;
import parser_wrappers_b2;
import gstd;
import parser_b2;

export namespace cp_consts {
    using SizeT = parser::wrappers::CpoolSizeT;
    using RefKindT = parser::wrappers::Number<uint8_t>;
    using TagIdTypeT = parser::wrappers::Number<opcodes::TagIdEnum>;

    namespace inst {
        template <opcodes::TagIdEnum tag, typename Prim, typename RawPrim = gstd::GetWidstructDefBaser<Prim>::Result>
        struct Primitive : parser::common::SerCombinatorPMxn<Primitive<tag, Prim, RawPrim>, Prim> {
            Prim value;

            TAG_FLAG(tag);

            explicit constexpr Primitive(Prim v) noexcept : value(v) {
            }

            constexpr Primitive() noexcept = default;

            NODISCARD std::string to_str() const noexcept {
                return std::to_string(static_cast<cstd::GetMinNumStrUType<RawPrim>>(gstd::unwrap(value)));
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                value.deser(writePtr);
                return {};
            }
        };

        struct Class : parser::common::SerCombinatorPMxn<Class, SizeT> { // NOLINT(*-pro-type-member-init)
            SizeT classIdx;

            explicit constexpr Class(const SizeT v) noexcept : classIdx(v) {
            }

            constexpr Class() noexcept = default;

            DEF_TAG_FLAG(CONSTANT_Class)

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(gstd::unwrap(classIdx));
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                reader::utils::deser_rev(writePtr, classIdx);
                return {};
            }
        };

        template <opcodes::TagIdEnum tag>
        struct MF : parser::common::SerCombinatorPMxn<MF<tag>, SizeT, SizeT> { // NOLINT(*-pro-type-member-init)
            SizeT classIdx;
            SizeT nameAndTypeIdx;

            TAG_FLAG(tag)

            constexpr MF(const SizeT classIdx, const SizeT nameAndTypeIdx) noexcept :
                classIdx(classIdx), nameAndTypeIdx(nameAndTypeIdx) {
            }

            constexpr MF() noexcept = default;

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(gstd::unwrap(classIdx)) + ".#" +
                       std::to_string(gstd::unwrap(nameAndTypeIdx));
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                classIdx.deser(writePtr);
                nameAndTypeIdx.deser(writePtr);
                return {};
            }
        };

        struct String : parser::common::SerCombinatorPMxn<String, SizeT> { // NOLINT(*-pro-type-member-init)
            SizeT stringIdx;

            DEF_TAG_FLAG(CONSTANT_String)

            explicit constexpr String(const SizeT stringIdx) noexcept : stringIdx(stringIdx) {
            }

            constexpr String() noexcept = default;

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(gstd::unwrap(stringIdx));
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                stringIdx.deser(writePtr);
                return {};
            }
        };

        template <opcodes::TagIdEnum tag>
        struct MP : parser::common::SerCombinatorPMxn<MP<tag>, SizeT> { // NOLINT(*-pro-type-member-init)
            SizeT nameIdx;

            TAG_FLAG(tag)

            explicit constexpr MP(const SizeT nameIdx) noexcept : nameIdx(nameIdx) {
            }

            constexpr MP() noexcept = default;

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                nameIdx.deser(writePtr);
                return {};
            }

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(gstd::unwrap(nameIdx));
            }
        };

        struct MethodHandle {
            SizeT refIdx;
            RefKindT refKind;

            DEF_TAG_FLAG(CONSTANT_MethodHandle)

            explicit constexpr MethodHandle(const SizeT refIdx, const RefKindT refKind) noexcept :
                refIdx(refIdx), refKind(refKind) {
            }

            constexpr MethodHandle() noexcept = default;

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return std::to_string(gstd::unwrap(refKind)) + ":#" + std::to_string(gstd::unwrap(refIdx));
            }

            template <typename SpecR>
            NODISCARD static constexpr std::optional<MethodHandle>
            parse(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                const auto refKind = RefKindT::parse(readPtr);
                const auto refIdx = SizeT::parse(readPtr);
                if (!(refKind && refIdx)) [[unlikely]]
                    return std::nullopt;
                return {MethodHandle{*refIdx, *refKind}};
            }

            template <typename SpecR>
            NODISCARD static constexpr bool check_and_move(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                return RefKindT::check_and_move(readPtr) && SizeT::check_and_move(readPtr);
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                refKind.deser(writePtr);
                refIdx.deser(writePtr);
                return {};
            }
        };

        struct NameAndType
            : parser::common::SerCombinatorPMxn<NameAndType, SizeT, SizeT> { // NOLINT(*-pro-type-member-init)
            SizeT nameIdx, descIdx;

            DEF_TAG_FLAG(CONSTANT_NameAndType)

            explicit constexpr NameAndType(const SizeT nameIdx, const SizeT descIdx) noexcept :
                nameIdx(nameIdx), descIdx(descIdx) {
            }

            constexpr NameAndType() noexcept = default;

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                nameIdx.deser(writePtr);
                descIdx.deser(writePtr);
                return {};
            }

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(gstd::unwrap(nameIdx)) + ":#" + std::to_string(gstd::unwrap(descIdx));
            }
        };

        struct MethodType : parser::common::SerCombinatorPMxn<MethodType, SizeT> { // NOLINT(*-pro-type-member-init)
            SizeT descIdx;

            DEF_TAG_FLAG(CONSTANT_MethodType)

            explicit constexpr MethodType(const SizeT descIdx) noexcept : descIdx(descIdx) {
            }

            constexpr MethodType() noexcept = default;

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                descIdx.deser(writePtr);
                return {};
            }

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(gstd::unwrap(descIdx));
            }
        };

        template <opcodes::TagIdEnum tag>
        struct Dynamic : parser::common::SerCombinatorPMxn<Dynamic<tag>, SizeT, SizeT> { // NOLINT(*-pro-type-member-init)
            SizeT bootstrapMethodAttrIdx, nameAndTypeIdx;

            TAG_FLAG(tag)

            explicit constexpr Dynamic(const SizeT bootstrapMethodAttrIdx, const SizeT nameAndTypeIdx) noexcept :
                bootstrapMethodAttrIdx(bootstrapMethodAttrIdx), nameAndTypeIdx(nameAndTypeIdx) {
            }

            constexpr Dynamic() noexcept = default;

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                bootstrapMethodAttrIdx.deser(writePtr);
                nameAndTypeIdx.deser(writePtr);
                return {};
            }

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(gstd::unwrap(bootstrapMethodAttrIdx)) + ":#" +
                       std::to_string(gstd::unwrap(nameAndTypeIdx));
            }
        };

        struct SourceUtf8 : parser::common::SerCombinatorPMxn<SourceUtf8, parser::common::RawJaacSpan<>> {
            parser::common::RawJaacSpan<> span{nullptr};

            DEF_TAG_FLAG(CONSTANT_Utf8)

            explicit constexpr SourceUtf8(const parser::common::RawJaacSpan<> span) noexcept :
                span(span) {
            }

            constexpr SourceUtf8() noexcept = default;

            template <typename SpecW>
            void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                return span.deser(writePtr);
            }

            NODISCARD std::string to_str() const noexcept {
                return {span.begin(), span.end()};
            }
        };

        struct CustomUtf8 : parser::common::SerCombinatorPMxn<CustomUtf8, parser::common::RawAnyJaacSpan<>> {
            parser::common::RawAnyJaacSpan<> span{};

            DEF_TAG_FLAG(CONSTANT_Utf8)

            explicit constexpr CustomUtf8(const parser::common::RawAnyJaacSpan<> span) noexcept :
                span(span) {
            }

            constexpr CustomUtf8() noexcept = default;

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                span.deser(writePtr);
                return {};
            }

            NODISCARD std::string to_str() const noexcept {
                return {span.begin(), span.end()};
            }
        };
    } // namespace inst

    namespace traits {
        using Integer = inst::Primitive<opcodes::TagIdEnum::CONSTANT_Integer, parser::wrappers::JInt>;
        using Float = inst::Primitive<opcodes::TagIdEnum::CONSTANT_Float, parser::wrappers::JFloat>;
        using Long = inst::Primitive<opcodes::TagIdEnum::CONSTANT_Long, parser::wrappers::JLong>;
        using Double = inst::Primitive<opcodes::TagIdEnum::CONSTANT_Double, parser::wrappers::JDouble>;
        using Class = inst::Class;
        using String = inst::String;
        using MethodHandle = inst::MethodHandle;
        using MethodType = inst::MethodType;
        using Dynamic = inst::Dynamic<opcodes::TagIdEnum::CONSTANT_Dynamic>;
        using Utf8 = inst::SourceUtf8;
        using Fieldref = inst::MF<opcodes::TagIdEnum::CONSTANT_Fieldref>;
        using Methodref = inst::MF<opcodes::TagIdEnum::CONSTANT_Methodref>;
        using InterfaceMethodref = inst::MF<opcodes::TagIdEnum::CONSTANT_InterfaceMethodref>;
        using NameAndType = inst::NameAndType;
        using InvokeDynamic = inst::Dynamic<opcodes::TagIdEnum::CONSTANT_InvokeDynamic>;
        using Module = inst::MP<opcodes::TagIdEnum::CONSTANT_Module>;
        using Package = inst::MP<opcodes::TagIdEnum::CONSTANT_Package>;

        using Utf8Custom = inst::CustomUtf8;

        struct EmptyVarValue {};

        using All = dstd::args::TypesTraits<Integer, Float, Long, Double, Class, String, MethodHandle, MethodType,
                                            Dynamic, Utf8, Fieldref, Methodref, InterfaceMethodref, NameAndType,
                                            InvokeDynamic, Module, Package>;

        using VariantAllUnempty = All::Append<Utf8Custom>;
        using VariantAll = VariantAllUnempty::Prepend<EmptyVarValue>;
    } // namespace traits

    template <typename LambdaRetType>
    CPP_KOSTYL std::optional<LambdaRetType> switch_tag_id_enum_2compl(const opcodes::TagIdEnum tagId,
                                                                      auto&& callable) noexcept {
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
                return std::nullopt;
        }
    }

    template <typename LambdaRetType>
    CPP_KOSTYL std::optional<LambdaRetType> switch_tag_id_enum_1compl_1rt(const opcodes::TagIdEnum tagId,
                                                                          auto&& callable) noexcept {
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
                return std::nullopt;
        }
    }

    template <typename LambdaRetType>
    CPP_KOSTYL std::optional<LambdaRetType> switch_tag_id_enum_1compl(const opcodes::TagIdEnum tagId,
                                                                      auto&& callable) noexcept {
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
                return std::nullopt;
        }
    }

    CPP_KOSTYL constexpr std::optional<std::string_view> tag_to_str_view(const opcodes::TagIdEnum tagId) noexcept {
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
        return name ? static_cast<std::string>(*name)
                    : "NA: " + std::to_string(static_cast<opcodes::TagIdEnumType>(tagId));
    }

    std::ostream& operator<<(std::ostream& stream, const opcodes::TagIdEnum tagId) {
        return stream << tag_to_str(tagId);
    }

    template <bool handle = false, typename SpecR>
    bool parse1_const_big_ctx(parser::ReadPtrBySpec<SpecR>& readPtr, auto&& handler) noexcept;

    namespace traits {
        struct cp_var : VariantAll::Wrap<cstd::emp_ignore_variant_of_ptrs_and_little_t_ptr> {
            template <typename SpecR>
            NODISCARD static cstd::var_opt<cp_var> parse(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                cp_var var;
                if (!parse1_const_big_ctx<true>(readPtr, [&](TagIdTypeT, const cp_var& value) { var = value; }))
                    return {};
                return var;
            }

            template <typename SpecR>
            NODISCARD static bool check_and_move(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                return parse1_const_big_ctx<false>(readPtr, [](TagIdTypeT) {});
            }

            template <typename SpecW, bool unlikely = false>
            NODISCARD gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) noexcept {
                if constexpr (CPP_KOSTYL unlikely) {
                    if (is_holds_empty()) [[unlikely]] {
                        return {};
                    }
                } else {
                    if (is_holds_empty()) {
                        return {};
                    }
                }
                deref_visit([&writePtr]<typename DirSelf>(DirSelf&& self) {
                    using Self = std::remove_cvref_t<DirSelf>;
                    Self::TAG_ID.deser(writePtr);
                    self.deser(writePtr);
                });
                return {};
            }
        };

        using opt_cp_var = cstd::var_opt<cp_var>;

        // ReSharper disable once CppPassValueParameterByConstReference
        std::string variant_to_str(const cp_var var) noexcept { // NOLINT(*-unnecessary-value-param)
            return var.is_holds_empty() ? var.deref_visit([](auto&& value) -> std::string { return value.to_str(); })
                                        : std::string{"EMPTY"};
        }
    } // namespace traits

    template <bool handle = false, typename SpecR>
    bool parse1_const_big_ctx(parser::ReadPtrBySpec<SpecR>& readPtr, auto&& handler) noexcept {
        DEF_OR_ELSE_RET_FALSE(TagIdTypeT, tagId);

        const auto handle_lambda = [&readPtr] {
            if constexpr (handle) {
                return [&readPtr]<opcodes::TagIdEnum, typename Trait>() noexcept -> traits::opt_cp_var {
                    if (auto value = Trait::parse(readPtr)) {
                        return {{*value}};
                    }
                    return {};
                };
            } else {
                return [&readPtr]<opcodes::TagIdEnum, typename Trait>() noexcept -> bool {
                    return Trait::check_and_move(readPtr);
                };
            }
        }();

        auto result = cp_consts::switch_tag_id_enum_2compl<std::conditional_t<handle, traits::opt_cp_var, bool>>(
                tagId, handle_lambda);

        DEF_ASSERT_FALSE(result);
        DEF_ASSERT_FALSE(*result);

        if constexpr (handle) {
            handler(tagId, **result);
        } else {
            handler(tagId);
        }

        return false;
    }
} // namespace cp_consts
