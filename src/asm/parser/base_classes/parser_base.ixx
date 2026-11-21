module;

#include <optional>
#include <span>
#include <vector>

#include "../../../cstd/macro.h"
#include "../parser_mcr.h"

export module parser_base;

import parser_mxns;
import parser_concepts;
import parser_utils;
import simple_cstd;
import parser_widstructs;
import cstd_variant;
import gstd;
import costd;
import macro_u;

export namespace parser::common {
    using namespace mxns;

    template <typename ElemT, typename SizeT = wrappers::ArraySizeT>
    struct JaacSpan : std::span<ElemT>, JaacSpanPMxn {
        using std::span<ElemT>::span;

        template <typename SpecW>
            requires JaacDeserable<SizeT, SpecW> && JaacDeserable<ElemT, SpecW>
        bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            ul_assert_false(static_cast<SizeT>(self.size()).deser(writePtr));
            for (auto& i : self)
                ul_assert_false(i.deser(writePtr));
            return true;
        }
    };

    template <typename RawElemT = uint8_t, typename SizeT = wrappers::ArraySizeT>
    struct RawAnyJaacSpan : std::span<RawElemT>, JaacSpanPMxn {
        using std::span<RawElemT>::span;

        IMPLICIT RawAnyJaacSpan(std::span<RawElemT> span) noexcept : std::span<RawElemT>(span) {
        }

        template <typename SpecW, typename Self>
        bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            ul_assert_false(static_cast<SizeT>(self.size()).deser(writePtr));
            writePtr.deser_range(std::forward<Self>(self));
            return true;
        }
    };

    struct JaacVectorTySettings {
        bool isIndepDeserPref = true;
        bool isIndepDeepParsePref = true;
    };

    template <typename ElemT, typename SizeT = wrappers::ArraySizeT, JaacVectorTySettings settings = {}>
    struct JaacVector : std::vector<ElemT>, JaacVectorPMxn {
        using std::vector<ElemT>::vector;

    protected:
        COM size_t deser_start_idx = 0;

    public:

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR> && JaacParsable<ElemT, SpecR>
        NODISCARD static std::optional<JaacVector> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            PARSE_OR_RET_NULLOPT(SizeT, typedSize);
            const auto size = static_cast<size_t>(typedSize);

            JaacVector self{};
            self.reserve(size);

            for (size_t i = 0; i < size; ++i) {
                /*opt<ElemT>*/ auto j = ElemT::parse(readPtr);
                ul_assert_nullopt(j);
                self.emplace_back(std::move(*j));
            }

            return std::optional{std::move(self)};
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR> && JaacParsable<ElemT, SpecR>
        NODISCARD static std::optional<JaacVector> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return parse(readPtr);
        }

        template <typename... Args, typename Self> requires JaacDeepParsable<ElemT, Args...> && (settings.isIndepDeepParsePref || !JaacDeepParsable<ElemT, Self&, size_t, Args...>)
        NODISCARD bool deep_parse(this Self&& self, Args&&... args) noexcept {
            for (auto& i : self) {
                ul_assert_false(i.deep_parse(std::forward<Args>(args)...));
            }
            return true;
        }

        template <typename... Args, typename Self> requires JaacDeepParsable<ElemT, Self&, size_t, Args...> && (!settings.isIndepDeepParsePref || !JaacDeepParsable<ElemT, Args...>)
        NODISCARD bool deep_parse(this Self&& self, Args&&... args) noexcept {
            const auto size = self.size();

            for (size_t i = 0; i < size; ++i) {
                ul_assert_false(self[i].deep_parse(self, i, std::forward<Args>(args)...));
            }

            return true;
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR> && JaacParsable<ElemT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            PARSE_OR_RET_FALSE(SizeT, typedSize);

            const auto size = static_cast<size_t>(typedSize);

            for (size_t i = 0; i < size; ++i) {
                ul_assert_false(ElemT::check_and_move(readPtr));
            }

            return true;
        }


        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR> && JaacParsable<ElemT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return check_and_move(readPtr);
        }

        template <typename SpecW, typename Self>
            requires JaacDeserable<SizeT, SpecW> && JaacDeserable<ElemT, SpecW> &&
                     (settings.isIndepDeserPref || !JaacInCollectionDeserable<ElemT, std::remove_cvref_t<Self>, SpecW>)
        bool deser(this Self&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            auto size = self.size();

            ul_assert_false(static_cast<SizeT>(size).deser(writePtr));

            for (size_t i = self.deser_start_idx; i < size; ++i) {
                ul_assert_false(self[i].deser(writePtr));
            }

            return true;
        }

        template <typename SpecW, typename Self>
            requires JaacDeserable<SizeT, SpecW> &&
                     JaacInCollectionDeserable<ElemT, std::remove_cvref_t<Self>, SpecW> &&
                     (!settings.isIndepDeserPref || !JaacDeserable<ElemT, SpecW>)
        bool deser(this Self&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            auto size = self.size();

            ul_assert_false(static_cast<SizeT>(size).deser(writePtr));

            for (size_t i = self.deser_start_idx; i < size; ++i) {
                ul_assert_false(self[i].deser(writePtr, i, self));
            }

            return true;
        }
    };

    template <typename RawElemT = uint8_t, typename SizeT = wrappers::ArraySizeT>
    struct RawJaacSpan : JaacSpanPMxn {
        u8_ptr sizePtr = nullptr;

        explicit constexpr RawJaacSpan(const u8_ptr sizePtr) noexcept : sizePtr(sizePtr) { // NOLINT(*-misplaced-const)
        }

        NODISCARD UncheckedReadPtr get_size_read_ptr() const noexcept {
            return UncheckedReadPtr{sizePtr};
        }

        NODISCARD UncheckedReadPtr get_after_size_read_ptr() const noexcept {
            auto readPtr = get_size_read_ptr();
            SizeT::parse(readPtr);
            return readPtr;
        }

        NODISCARD u8_ptr get_after_size_ptr() const noexcept {
            return get_after_size_read_ptr().i;
        }

        NODISCARD u8_ptr get_end_ptr() const noexcept {
            auto readPtr = get_size_read_ptr();
            const auto size = static_cast<size_t>(*SizeT::parse(readPtr));
            DISCARD readPtr.check_and_move(sizeof(RawElemT) * size);
            return readPtr.i;
        }

        NODISCARD SizeT typed_size() const noexcept {
            return *SizeT::parse(get_size_read_ptr());
        }

        NODISCARD size_t size() const noexcept {
            return static_cast<size_t>(typed_size());
        }

        NODISCARD u8_ptr begin() const noexcept {
            return get_after_size_ptr();
        }

        NODISCARD u8_ptr end() const noexcept {
            return get_end_ptr();
        }

        NODISCARD u8_ptr cbegin() const noexcept {
            return begin();
        }

        NODISCARD u8_ptr cend() const noexcept {
            return end();
        }

        NODISCARD UncheckedReadPtr get_data_interval() const noexcept {
            return UncheckedReadPtr{begin(), end()};
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            PARSE_OR_RET_FALSE(SizeT, typedSize);
            ul_assert_false(readPtr.check_and_move(sizeof(RawElemT) * static_cast<size_t>(*typedSize)));
            return true;
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            PARSE_OR_RET_FALSE(SizeT, typedSize);
            ul_assert_false(std::move(readPtr).check_and_move(sizeof(RawElemT) * static_cast<size_t>(*typedSize)));
            return true;
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static std::optional<RawJaacSpan> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            auto sizePtr = readPtr.i;
            ul_assert_nullopt(check_and_move(readPtr));
            return std::optional{RawJaacSpan{sizePtr}};
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static std::optional<RawJaacSpan> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            auto sizePtr = readPtr.i;
            ul_assert_nullopt(check_and_move(std::move(readPtr)));
            return std::optional{RawJaacSpan{sizePtr}};
        }

        template <typename SpecW>
            requires JaacDeserable<SizeT, SpecW>
        gstd::true_void deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            writePtr.deser_range(UncheckedReadPtr{self.sizePtr, self.get_end_ptr()});
            return {};
        }
    };

    template <typename RawElemT = uint8_t, typename SizeT = ArrayRSizeT>
    struct RawFullCachedJaacSpan : JaacSpanPMxn {
        u8_ptr dataBegin = nullptr;
        u8_ptr dataEnd = nullptr;
        u8_ptr sizePtr = nullptr;
        SizeT typedSize{};

        constexpr RawFullCachedJaacSpan(const u8_ptr begin, // NOLINT(*-pro-type-member-init, *-misplaced-const)
                                        const u8_ptr end, // NOLINT(*-pro-type-member-init, *-misplaced-const)
                                        const u8_ptr sizePtr, // NOLINT(*-pro-type-member-init, *-misplaced-const)
                                        SizeT size) noexcept :
            dataBegin(begin), dataEnd(end), sizePtr(sizePtr), typedSize(size) {
        }

        constexpr RawFullCachedJaacSpan() noexcept = default;

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static std::optional<RawFullCachedJaacSpan> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            auto sizePtr = readPtr.i;
            PARSE_OR_RET_NULLOPT(SizeT, typedSize);
            const auto size = static_cast<size_t>(typedSize);

            u8_ptr first = readPtr.i;
            ul_assert_nullopt(readPtr.check_and_move(sizeof(RawElemT) * size));

            return std::optional{RawFullCachedJaacSpan{first, readPtr.i, sizePtr, typedSize}};
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static std::optional<RawFullCachedJaacSpan> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return parse(readPtr);
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return RawJaacSpan<RawElemT, SizeT>::check_and_move(readPtr);
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return RawJaacSpan<RawElemT, SizeT>::check_and_move(std::move(readPtr));
        }

        template <typename SpecW, typename Self>
            requires JaacDeserable<SizeT, SpecW>
        bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            ul_assert_false(std::forward<Self>(self).typedSize.deser(writePtr));
            writePtr.deser_range(std::forward<Self>(self));
            return true;
        }

        NODISCARD SizeT typed_size() const noexcept {
            return typedSize;
        }

        NODISCARD size_t size() const noexcept {
            return static_cast<size_t>(typedSize);
        }

        NODISCARD u8_ptr begin() const noexcept {
            return dataBegin;
        }

        NODISCARD u8_ptr end() const noexcept {
            return dataEnd;
        }

        NODISCARD u8_ptr cbegin() const noexcept {
            return begin();
        }

        NODISCARD u8_ptr cend() const noexcept {
            return end();
        }

        IMPLICIT operator RawJaacSpan<RawElemT, SizeT>() noexcept {
            return RawJaacSpan<RawElemT, SizeT>{sizePtr};
        }
    };

    namespace internal {
        template <typename AttrsSpan>
        struct FullAttributeMethodsPart {
            template <typename SpecW, typename Self>
                requires JaacDeserable<AttrsSpan, SpecW>
            bool deser(this Self&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                ul_assert_false(self.get_attr_name_idx().deser(writePtr));
                ul_assert_false(cstd::forward_static_cast<AttrsSpan>(std::forward<Self>(self)).deser(writePtr));
                return true;
            }
        };
    } // namespace internal

    template <typename AttrsSpan = RawJaacSpan<uint8_t, wrappers::AttrsInnerSizeT>>
    struct FullAttribute;

    using DefVeryFullAttribute = FullAttribute<RawFullCachedJaacSpan<uint8_t, wrappers::AttrsInnerSizeT>>;

    template <typename AttrSpan = RawJaacSpan<uint8_t, wrappers::AttrsInnerSizeT>>
    struct FullAttribute : AttrSpan,
                           SerCombinatorPMxn<FullAttribute<AttrSpan>, wrappers::CpoolSizeT, AttrSpan>,
                           internal::FullAttributeMethodsPart<AttrSpan> {
        using AttrSpan::AttrSpan;

        wrappers::CpoolSizeT attrNameIdx{};

        constexpr FullAttribute(const wrappers::CpoolSizeT name, AttrSpan& span) noexcept :
            AttrSpan(span), attrNameIdx(name) {
        }

        constexpr FullAttribute(const wrappers::CpoolSizeT name, AttrSpan&& span) noexcept :
            AttrSpan(std::move(span)), attrNameIdx(name) {
        }

        IMPLICIT constexpr FullAttribute(DefVeryFullAttribute& veryFull) noexcept
            requires(!std::same_as<FullAttribute, DefVeryFullAttribute>)
            : AttrSpan(veryFull), attrNameIdx(veryFull.attrNameIdx) {
        }

        IMPLICIT constexpr FullAttribute(DefVeryFullAttribute&& veryFull) noexcept
            requires(!std::same_as<FullAttribute, DefVeryFullAttribute>)
            : AttrSpan(veryFull), attrNameIdx(veryFull.attrNameIdx) {
        }

        NODISCARD decltype(auto) get_attr_name_idx(this auto&& self) noexcept {
            return std::forward_like<decltype(self)>(self.attrNameIdx);
        }

    private:
        using Base2 = SerCombinatorPMxn<FullAttribute, wrappers::CpoolSizeT, AttrSpan>;
        using Base3 = internal::FullAttributeMethodsPart<AttrSpan>;

    public:
        using Base3::deser;
        using Base2::parse;
        using Base2::check_and_move;
    };

    template <typename RawElemT = uint8_t, typename SizeT = wrappers::ArraySizeT>
    struct RawFullAttribute : RawJaacSpan<RawElemT, SizeT>,
                           SerCombinatorPMxn<RawFullAttribute<RawElemT, SizeT>, wrappers::CpoolSizeT, RawJaacSpan<RawElemT, SizeT>>,
                           internal::FullAttributeMethodsPart<RawJaacSpan<RawElemT, SizeT>> {
        using RawJaacSpan<RawElemT, SizeT>::RawJaacSpan;

    private:
        using AttrSpan = RawJaacSpan<RawElemT, SizeT>;

        constexpr RawFullAttribute(const wrappers::CpoolSizeT, AttrSpan& span) : RawFullAttribute(span) {
        }

        constexpr RawFullAttribute(const wrappers::CpoolSizeT, AttrSpan&& span) : RawFullAttribute(std::move(span)) {
        }

    public:
        IMPLICIT constexpr RawFullAttribute(AttrSpan& span) noexcept :
            AttrSpan(span) {
        }

        IMPLICIT constexpr RawFullAttribute(AttrSpan&& span) noexcept :
            AttrSpan(std::move(span)) {
        }

        NODISCARD constexpr auto get_attr_name_idx(this auto&& self) noexcept {
            return *wrappers::CpoolSizeT::parse(UncheckedReadPtr{self.sizePtr - sizeof(wrappers::CpoolSizeT)});
        }

        IMPLICIT constexpr operator FullAttribute<AttrSpan>() noexcept {
            return FullAttribute<AttrSpan>{this->get_attr_name_idx(), static_cast<AttrSpan>(*this)};
        }

    private:
        using Base3 = internal::FullAttributeMethodsPart<AttrSpan>;
        using Base2 = SerCombinatorPMxn<RawFullAttribute, wrappers::CpoolSizeT, AttrSpan>;

    public:
        using Base3::deser;
        using Base2::parse;
        using Base2::check_and_move;

        friend Base2;
    };

    namespace internal {
        template <typename AttrSpan>
        struct MinAttributeTrait {
            using Result = FullAttribute<AttrSpan>;
        };

        template <typename RawElemT, typename SizeT>
        struct MinAttributeTrait<RawJaacSpan<RawElemT, SizeT>> {
            using Result = RawFullAttribute<RawElemT, SizeT>;
        };
    }

    template <typename AttrSpan = RawJaacSpan<uint8_t, wrappers::AttrsInnerSizeT>>
    using MinAttribute = internal::MinAttributeTrait<AttrSpan>::Result;

    using DefAttrsCont = JaacVector<MinAttribute<>>;

    template <typename RawElemT, typename SizeT>
    NODISCARD constexpr auto unchecked_cast_to_raw_full_attr(FullAttribute<RawJaacSpan<RawElemT, SizeT>> attr) noexcept {
        return RawFullAttribute<RawElemT, SizeT>(static_cast<RawJaacSpan<RawElemT, SizeT>>(attr));
    }

    template <typename RawElemT, typename SizeT>
    NODISCARD constexpr auto unchecked_cast_to_raw_full_attr(RawFullAttribute<RawElemT, SizeT> attr) noexcept {
        return attr;
    }

    template <typename AttrsCont = DefAttrsCont>
    struct AttrsComponent : SerCombinatorPMxn<AttrsComponent<AttrsCont>, AttrsCont> {
        AttrsCont attrs{};

        explicit constexpr AttrsComponent(AttrsCont& attrs) noexcept : attrs(attrs) {
        }

        explicit constexpr AttrsComponent(AttrsCont&& attrs) noexcept : attrs(std::move(attrs)) {
        }

        constexpr AttrsComponent() noexcept = default;

        template <typename SpecW, typename Self>
            requires JaacDeserable<AttrsCont, SpecW>
        auto deser(this Self&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            return std::forward<Self>(self).attrs.deser(writePtr);
        }

        using SerCombinatorPMxn<AttrsComponent, AttrsCont>::parse;
        using SerCombinatorPMxn<AttrsComponent, AttrsCont>::check_and_move;
    };

    template <typename Base, typename AttrsCont = DefAttrsCont>
    using HasAttrWMxn = AddPostfieldsWMxn<Base, AttrsComponent<AttrsCont>>;

    namespace internal {
        struct MFInfoNoattrDeserPart {
            template <typename SpecW, typename Self>
            bool deser(this Self&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                ul_assert_false(std::forward<Self>(self).accessFlags.deser(writePtr));
                ul_assert_false(std::forward<Self>(self).nameIdx.deser(writePtr));
                ul_assert_false(std::forward<Self>(self).descriptorIdx.deser(writePtr));
                return true;
            }
        };
    } // namespace internal

    struct MFInfoNoattr
        : SerCombinatorPMxn<MFInfoNoattr, wrappers::AccessFlags, wrappers::CpoolSizeT, wrappers::CpoolSizeT>,
          internal::MFInfoNoattrDeserPart {
        wrappers::AccessFlags accessFlags{};
        wrappers::CpoolSizeT nameIdx{}, descriptorIdx{};

        constexpr MFInfoNoattr(const wrappers::AccessFlags accessFlags, const wrappers::CpoolSizeT nameIdx,
                               const wrappers::CpoolSizeT descriptorIdx) noexcept :
            accessFlags(accessFlags), nameIdx(nameIdx), descriptorIdx(descriptorIdx) {
        }

        constexpr MFInfoNoattr() noexcept = default;

        using MFInfoNoattrDeserPart::deser;
        using SerCombinatorPMxn::check_and_move;
        using SerCombinatorPMxn::parse;
    };

    template <typename AttrsCont = DefAttrsCont>
    using MfInfoUnwrapped = HasAttrWMxn<MFInfoNoattr, AttrsCont>;

    template <typename AttrsCont = DefAttrsCont>
    struct MFInfo : JaacWidstruct<MfInfoUnwrapped<AttrsCont>, MFInfo<AttrsCont>> {
        using JaacWidstruct<MfInfoUnwrapped<AttrsCont>, MFInfo>::JaacWidstruct;
        friend RewrapperPMxn<MfInfoUnwrapped<AttrsCont>, MFInfo>;

    private:
        using Base = JaacWidstruct<MfInfoUnwrapped<AttrsCont>, MFInfo>;
        using UnwrBase1 = MfInfoUnwrapped<>;

    public:
        template <typename T> requires cstd::is_implicit_convertable<T, AttrsCont>
        IMPLICIT MFInfo(MFInfo<T>& other) noexcept : Base(other.accessFlags, other.nameIdx, other.descriptorIdx) {
            this->attrs = other.attrs;
        }

        template <typename T> requires cstd::is_implicit_convertable<T, AttrsCont>
        IMPLICIT MFInfo(MFInfo<T>&& other) noexcept : Base(other.accessFlags, other.nameIdx, other.descriptorIdx) {
            this->attrs = std::move(other).attrs;
        }

        template <typename... Args>
            requires JaacDeepParsable<AttrsCont, Args...> && (!JaacDeepParsable<AttrsCont, MFInfoNoattr&, Args...>)
        bool deep_parse(Args&&... args) noexcept {
            return this->attrs.deep_parse(std::forward<Args>(args)...);
        }

        template <typename... Args>
            requires JaacDeepParsable<AttrsCont, MFInfoNoattr&, Args...>
        bool deep_parse(Args&&... args) noexcept {
            return this->attrs.deep_parse(static_cast<MFInfoNoattr&>(*this), std::forward<Args>(args)...);
        }
    };
} // namespace parser::common
