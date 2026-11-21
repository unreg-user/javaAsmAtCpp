module;

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

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
import parser_widstructs;
import gstd;
import parser_base;
import parser_mxns;
import macro_u;
import parser_concepts;

export namespace cp_consts {
    using SizeT = parser::wrappers::CpoolSizeT;
    using RefKindT = parser::wrappers::Number<uint8_t>;
    using TagIdTypeT = parser::wrappers::Number<opcodes::TagIdEnum>;

    namespace inst {
        template <opcodes::TagIdEnum tag, typename Prim, typename RawPrim = gstd::GetWidTypeOrVoid<Prim>>
        struct Primitive : parser::mxns::SerCombinatorPMxn<Primitive<tag, Prim, RawPrim>, Prim> {
            Prim value{};

            TAG_FLAG(tag);

            explicit constexpr Primitive(Prim v) noexcept : value(v) {
            }

            constexpr Primitive() noexcept = default;

            NODISCARD std::string to_str() const noexcept {
                return std::to_string(static_cast<cstd::GetMinNumStrUType<RawPrim>>(value));
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                value.deser(writePtr);
                return {};
            }
        };

        struct Class : parser::mxns::SerCombinatorPMxn<Class, SizeT> {
            SizeT classIdx{};

            explicit constexpr Class(const SizeT v) noexcept : classIdx(v) {
            }

            constexpr Class() noexcept = default;

            DEF_TAG_FLAG(CONSTANT_Class)

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(classIdx);
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                classIdx.deser(writePtr);
                return {};
            }
        };

        template <opcodes::TagIdEnum tag>
        struct MF : parser::mxns::SerCombinatorPMxn<MF<tag>, SizeT, SizeT> {
            SizeT classIdx{};
            SizeT nameAndTypeIdx{};

            TAG_FLAG(tag)

            constexpr MF(const SizeT classIdx, const SizeT nameAndTypeIdx) noexcept :
                classIdx(classIdx), nameAndTypeIdx(nameAndTypeIdx) {
            }

            constexpr MF() noexcept = default;

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(classIdx) + ".#" + std::to_string(nameAndTypeIdx);
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                classIdx.deser(writePtr);
                nameAndTypeIdx.deser(writePtr);
                return {};
            }
        };

        struct String : parser::mxns::SerCombinatorPMxn<String, SizeT> {
            SizeT stringIdx{};

            DEF_TAG_FLAG(CONSTANT_String)

            explicit constexpr String(const SizeT stringIdx) noexcept : stringIdx(stringIdx) {
            }

            constexpr String() noexcept = default;

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return "#" + std::to_string(stringIdx);
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                stringIdx.deser(writePtr);
                return {};
            }
        };

        template <opcodes::TagIdEnum tag>
        struct MP : parser::mxns::SerCombinatorPMxn<MP<tag>, SizeT> {
            SizeT nameIdx{};

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
                return "#" + std::to_string(nameIdx);
            }
        };

        struct MethodHandle {
            SizeT refIdx{};
            RefKindT refKind{};

            DEF_TAG_FLAG(CONSTANT_MethodHandle)

            explicit constexpr MethodHandle(const SizeT refIdx, const RefKindT refKind) noexcept :
                refIdx(refIdx), refKind(refKind) {
            }

            constexpr MethodHandle() noexcept = default;

            NODISCARD std::string to_str() const noexcept { // NOLINT(*-convert-member-functions-to-static)
                return std::to_string(refKind) + ":#" + std::to_string(refIdx);
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
            NODISCARD static constexpr std::optional<MethodHandle>
            parse(parser::ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                const auto refKind = RefKindT::parse(readPtr);
                const auto refIdx = SizeT::parse(std::move(readPtr));
                if (!(refKind && refIdx)) [[unlikely]]
                    return std::nullopt;
                return {MethodHandle{*refIdx, *refKind}};
            }

            template <typename SpecR>
            NODISCARD static constexpr bool check_and_move(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                return RefKindT::check_and_move(readPtr) && SizeT::check_and_move(readPtr);
            }

            template <typename SpecR>
            NODISCARD static constexpr bool check_and_move(parser::ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                return RefKindT::check_and_move(readPtr) && SizeT::check_and_move(std::move(readPtr));
            }

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                refKind.deser(writePtr);
                refIdx.deser(writePtr);
                return {};
            }
        };

        struct NameAndType : parser::mxns::SerCombinatorPMxn<NameAndType, SizeT, SizeT> {
            SizeT nameIdx{}, descIdx{};

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
                return "#" + std::to_string(nameIdx) + ":#" + std::to_string(descIdx);
            }
        };

        struct MethodType : parser::mxns::SerCombinatorPMxn<MethodType, SizeT> {
            SizeT descIdx{};

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
                return "#" + std::to_string(descIdx);
            }
        };

        template <opcodes::TagIdEnum tag>
        struct Dynamic : parser::mxns::SerCombinatorPMxn<Dynamic<tag>, SizeT, SizeT> {
            SizeT bootstrapMethodAttrIdx{}, nameAndTypeIdx{};

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
                return "#" + std::to_string(bootstrapMethodAttrIdx) + ":#" + std::to_string(nameAndTypeIdx);
            }
        };

        struct SourceUtf8 : parser::mxns::SerCombinatorPMxn<SourceUtf8, parser::common::RawJaacSpan<>> {
            parser::common::RawJaacSpan<> span{nullptr};

            DEF_TAG_FLAG(CONSTANT_Utf8)

            explicit constexpr SourceUtf8(const parser::common::RawJaacSpan<> span) noexcept : span(span) {
            }

            constexpr SourceUtf8() noexcept = default;

            template <typename SpecW>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept {
                span.deser(writePtr);
                return {};
            }

            NODISCARD std::string to_str() const noexcept {
                return {span.begin(), span.end()};
            }
        };

        struct CustomUtf8 : parser::mxns::SerCombinatorPMxn<CustomUtf8, parser::common::RawAnyJaacSpan<>> {
            parser::common::RawAnyJaacSpan<> span{};

            DEF_TAG_FLAG(CONSTANT_Utf8)

            explicit constexpr CustomUtf8(const parser::common::RawAnyJaacSpan<> span) noexcept : span(span) {
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
    CPP_KOSTYL std::optional<LambdaRetType> trait_switch_tag_id_enum(const opcodes::TagIdEnum tagId,
                                                                     auto&& callable) noexcept {
        switch (tagId) {
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Integer, Integer); // NOLINT(*-branch-clone)
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Float, Float);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Long, Long);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Double, Double);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Class, Class);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_String, String);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_MethodHandle, MethodHandle);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_MethodType, MethodType);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Dynamic, Dynamic);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Utf8, Utf8);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Fieldref, Fieldref);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Methodref, Methodref);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_InterfaceMethodref, InterfaceMethodref);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_NameAndType, NameAndType);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_InvokeDynamic, InvokeDynamic);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Module, Module);
            SWITCH_TAG_CASE_GEN(opcodes::TagIdEnum::CONSTANT_Package, Package);
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

    template <bool handle = false, typename DirReadPtr>
    bool parse1_const_big_ctx(DirReadPtr&& readPtr, auto&& handler) noexcept;

    namespace traits {
        struct cp_var : VariantAll::Wrap<cstd::emp_ignore_variant_of_ptrs_and_little_t_ptr> {
            using VariantAll::Wrap<cstd::emp_ignore_variant_of_ptrs_and_little_t_ptr>::Wrap;

            template <typename SpecR>
            NODISCARD static cstd::var_opt<cp_var> parse(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                cp_var var;
                if (!parse1_const_big_ctx<true>(readPtr, [&](TagIdTypeT, const cp_var& value) { var = value; }))
                    return {};
                return var;
            }

            template <typename SpecR>
            NODISCARD static cstd::var_opt<cp_var> parse(parser::ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                cp_var var;
                if (!parse1_const_big_ctx<true>(std::move(readPtr),
                                                [&](TagIdTypeT, const cp_var& value) { var = value; }))
                    return {};
                return var;
            }

            template <typename SpecR>
            NODISCARD static bool check_and_move(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                return parse1_const_big_ctx<false>(readPtr, [](TagIdTypeT) {});
            }

            template <typename SpecR>
            NODISCARD static bool check_and_move(parser::ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                return parse1_const_big_ctx<false>(std::move(readPtr), [](TagIdTypeT) {});
            }

            template <typename SpecW, bool unlikely = false>
            gstd::true_void deser(parser::WritePtrBySpec<SpecW>& writePtr) noexcept {
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
            return !var.is_holds_empty() ? var.visit([](auto&& value) -> std::string { return value.to_str(); })
                                         : std::string{"EMPTY"};
        }
    } // namespace traits

    template <bool handle = false, typename DirReadPtr>
    bool parse1_const_big_ctx(DirReadPtr&& readPtr, auto&& handler) noexcept {
        PARSE_OR_RET_FALSE(TagIdTypeT, tagId);

        auto handle_lambda = [&readPtr] <typename Trait> () mutable {
            if constexpr (handle) {
                if (auto value = Trait::parse(std::forward<DirReadPtr>(readPtr))) {
                    return traits::opt_cp_var{traits::cp_var{*value}};
                }
                return traits::opt_cp_var{};
            } else {
                return Trait::check_and_move(std::forward<DirReadPtr>(readPtr));
            }
        };

        auto result = cp_consts::trait_switch_tag_id_enum<std::conditional_t<handle, traits::opt_cp_var, bool>>(
                tagId, std::move(handle_lambda));

        ul_assert_false(result);
        ul_assert_false(*result);

        if constexpr (handle) {
            handler(tagId, **result);
        } else {
            handler(tagId);
        }

        return true;
    }

    template <parser::ParseJaacComplexCollMode mode, typename SpecR>
    NODISCARD constexpr bool opt_parse_cpool_coll(parser::ReadPtrBySpec<SpecR>& readPtr, const parser::CpoolRSizeT cpoolSize,
                                                  auto& cpoolJaacColl) noexcept {
        using namespace parser;

        auto skipIdxInColl = [&](bool& isUnex) {
            if constexpr (mode == ParseJaacComplexCollMode::PARSE) {
                isUnex = isUnex || !cpoolJaacColl.jaac_emplace_back(traits::cp_var{});
            } else if constexpr (mode == ParseJaacComplexCollMode::SKIP_AND_SAVE_INTERVALS) {
                isUnex = isUnex || !cpoolJaacColl.jaac_emplace_back(common::RawAnyJaacSpan<>{});
            }
        };

        {
            bool isUnex = false;
            skipIdxInColl(isUnex);
            ul_assert_false(!isUnex);
        }

        for (CpoolRSizeT i = 1; i < static_cast<CpoolRSizeT>(cpoolSize); ++i) {
            u8_ptr iOld = readPtr.i;
            bool isUnex = false;
            auto skipLDIdx = [&](const opcodes::TagIdEnum tagId) {
                if (tagId == opcodes::TagIdEnum::CONSTANT_Double || tagId == opcodes::TagIdEnum::CONSTANT_Long) {
                    skipIdxInColl(isUnex);
                    i++;
                }
            };
            COM_BOOL parseCpoolStructureFlag = (mode == ParseJaacComplexCollMode::PARSE);
            isUnex = isUnex ||
                     !cp_consts::parse1_const_big_ctx<parseCpoolStructureFlag>(
                             readPtr,
                             cstd::overload{[&](const opcodes::TagIdEnum tagId, traits::cp_var var) {
                                                if constexpr (parseCpoolStructureFlag) {
                                                    isUnex = !cpoolJaacColl.jaac_emplace_back(var);
                                                    skipLDIdx(tagId);
                                                }
                                            },
                                            [&](const opcodes::TagIdEnum tagId) {
                                                if constexpr (!parseCpoolStructureFlag) {
                                                    isUnex = !cpoolJaacColl.jaac_emplace_back(common::RawAnyJaacSpan{
                                                            static_cast<u8_span>(UncheckedReadPtr{iOld, readPtr.i})});
                                                    skipLDIdx(tagId);
                                                }
                                            }});
            ul_assert_false(!isUnex);
        }
        return true;
    }

    struct JaacCpoolVectorTySettings : parser::common::JaacVectorTySettings {
        bool parseCpoolStructure = false;
    };

    namespace internal {
        template <typename ElemT, JaacCpoolVectorTySettings settings, typename Self>
        struct CpoolJaacVectorMethodsPMxn {
        private:
            using SizeT = parser::wrappers::CpoolSizeT;

        public:
            template <typename SpecR>
            NODISCARD static std::optional<Self> parse(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                PARSE_N_UNWRAP_OR_RET_NULLOPT(SizeT, size);
                Self self{};
                ul_assert_nullopt(
                        opt_parse_cpool_coll<settings.parseCpoolStructure
                                                     ? parser::ParseJaacComplexCollMode::PARSE
                                                     : parser::ParseJaacComplexCollMode::SKIP_AND_SAVE_INTERVALS>(
                                readPtr, size, self));
                return self;
            }

            template <typename SpecR>
            NODISCARD static std::optional<Self> parse(parser::ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                return parse(readPtr);
            }

            template <typename SpecR>
            NODISCARD static bool check_and_move(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept {
                PARSE_OR_RET_FALSE(SizeT, size);
                ul_assert_false(opt_parse_cpool_coll<parser::ParseJaacComplexCollMode::FULL_SKIP>(readPtr, size,
                                                                                                  cstd::deleted));
                return true;
            }


            template <typename SpecR>
            NODISCARD static bool check_and_move(parser::ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                return check_and_move(readPtr);
            }
        };
    } // namespace internal

    template <typename ElemT, JaacCpoolVectorTySettings settings = {}>
    struct CpoolJaacVector : parser::common::JaacVector<ElemT, parser::wrappers::CpoolSizeT, settings>,
                             internal::CpoolJaacVectorMethodsPMxn<ElemT, settings, CpoolJaacVector<ElemT, settings>> {
        using parser::common::JaacVector<ElemT, parser::wrappers::CpoolSizeT, settings>::JaacVector;

    protected:
        COM size_t deser_start_idx = 1;

    private:
        using Base3 = internal::CpoolJaacVectorMethodsPMxn<ElemT, settings, CpoolJaacVector>;

    public:
        using Base3::check_and_move;
        using Base3::parse;
    };
} // namespace cp_consts
