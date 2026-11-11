module;

#include <optional>
#include <span>
#include <tuple>
#include <vector>

#include "../../../cstd/macro.h"

export module parser_b2;

import parser_b1;
import parser_concepts;
import read_utils;
import parser_utils;
import simple_cstd;
import parser_wrappers_b2;
import cstd_variant;
import gstd;

export namespace parser::common {
    template <typename Self, typename... Types>
    struct SerCombinatorPMxn {
        template <typename SpecR>
            requires(... && JaacParsable<Types, SpecR>)
        NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            if (auto parsedTuple = std::tuple{Types::parse(readPtr)...};
                !std::apply([](auto&&... parsed) { return (... && parsed); }, parsedTuple)) [[unlikely]] {
                return std::nullopt;
            } else {
                return std::apply(
                        []<typename... Parsed>(Parsed&&... parsed) -> std::optional<Self> {
                            return std::optional<Self>{std::in_place, (*std::forward<Parsed>(parsed))...};
                        },
                        std::move(parsedTuple));
            }
        }

        template <typename SpecR>
            requires(... && JaacParsable<Types, SpecR>)
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return (... && Types::check_and_move(readPtr));
        }
    };

    namespace internal {
        template <typename Base, typename PrefieldsSt, typename Self>
        struct AddPrefieldsWMxnMethodsPart {
            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PrefieldsSt, SpecR>
            NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
                auto prefSt = PrefieldsSt::parse(readPtr);
                auto base = Base::parse(readPtr);
                if (!(prefSt && base)) [[unlikely]] {
                    return std::nullopt;
                }
                return std::optional{Self{*prefSt, *base}};
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PrefieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
                return PrefieldsSt::check_and_move(readPtr) && Base::check_and_move(readPtr);
            }

            template <typename SpecW>
                requires JaacDeserable<Base, SpecW> && JaacDeserable<PrefieldsSt, SpecW>
            bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                return static_cast<PrefieldsSt>(self).deser(writePtr) && static_cast<Base>(self).deser(writePtr);
            }
        };
    } // namespace internal

    template <typename Base, typename PrefieldsSt>
    struct AddPrefieldsWMxn
        : PrefieldsSt,
          Base,
          internal::AddPrefieldsWMxnMethodsPart<Base, PrefieldsSt, AddPrefieldsWMxn<Base, PrefieldsSt>> {
    private:
        using Base2 = PrefieldsSt;
        using Base3 = internal::AddPrefieldsWMxnMethodsPart<Base, PrefieldsSt, AddPrefieldsWMxn>;
        friend Base3;

        constexpr AddPrefieldsWMxn(PrefieldsSt prefieldsStruct, Base base) noexcept :
            Base2(prefieldsStruct), Base(base) {
        }

    public:
        using Base::Base;

        using Base3::check_and_move;
        using Base3::deser;
        using Base3::parse;
    };

    namespace internal {
        template <typename Base, typename PostfieldsSt, typename Self>
        struct AddPostfieldsWMxnMethodsPart {
            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PostfieldsSt, SpecR>
            NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
                /*opt<Base>*/ auto base = Base::parse(readPtr);
                /*opt<PostfieldsSt>*/ auto postSt = PostfieldsSt::parse(readPtr);
                if (!(base && postSt)) [[unlikely]]
                    return std::nullopt;
                return std::optional{Self{*base, *postSt}};
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PostfieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
                // const bool b1 = PostfieldsSt::check_and_move(readPtr);
                // const bool b2 = Base::check_and_move(readPtr);
                // const bool ret_val = /**/ b1 && b2;
                return Base::check_and_move(readPtr) && PostfieldsSt::check_and_move(readPtr);
            }

            template <typename SpecW>
                requires JaacDeserable<Base, SpecW> && JaacDeserable<PostfieldsSt, SpecW>
            bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                return static_cast<Base>(self).deser(writePtr) && static_cast<PostfieldsSt>(self).deser(writePtr);
            }
        };
    } // namespace internal

    template <typename Base, typename PostfieldsSt>
    struct AddPostfieldsWMxn
        : Base,
          PostfieldsSt,
          internal::AddPostfieldsWMxnMethodsPart<Base, PostfieldsSt, AddPostfieldsWMxn<Base, PostfieldsSt>> {
    private:
        using Base2 = PostfieldsSt;
        using Base3 = internal::AddPostfieldsWMxnMethodsPart<Base, PostfieldsSt, AddPostfieldsWMxn>;
        friend Base3;

        constexpr AddPostfieldsWMxn(Base base, PostfieldsSt prefieldsStruct) noexcept :
            Base(base), Base2(prefieldsStruct), Base3() {
        }

    public:
        using Base::Base;

        using Base3::check_and_move;
        using Base3::deser;
        using Base3::parse;
    };

    template <typename ElemT, typename SizeT = wrappers::ArraySizeT>
    struct JaacSpan : std::span<ElemT>, JaacSpanPMxn {
        using std::span<ElemT>::span;

        template <typename SpecW>
            requires JaacDeserable<ElemT, SpecW> && JaacDeserable<ElemT, SizeT>
        bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            if (!static_cast<SizeT>(self.size()).deser(writePtr)) [[unlikely]]
                return false;
            for (auto& i : self)
                if (!i.deser(writePtr)) [[unlikely]]
                    return false;
            return true;
        }
    };

    template <typename RawElemT = uint8_t, typename SizeT = wrappers::ArraySizeT>
    struct RawAnyJaacSpan : std::span<RawElemT>, JaacSpanPMxn {
        using std::span<RawElemT>::span;

        /*implicit*/ RawAnyJaacSpan(std::span<RawElemT> span) noexcept : std::span<RawElemT>(span) {
        }

        template <typename SpecW>
        bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            if (!static_cast<SizeT>(self.size()).deser(writePtr)) [[unlikely]]
                return false;

            writePtr.deser_range(self);

            return true;
        }
    };

    struct JaacVectorTySettings {
        bool isIndepPref = true;
    };

    template <typename ElemT, typename SizeT = wrappers::ArraySizeT, JaacVectorTySettings settings = {}>
    struct JaacVector : std::vector<ElemT>, JaacVectorPMxn {
        using std::vector<ElemT>::vector;

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR> && JaacParsable<ElemT, SpecR>
        NODISCARD static std::optional<JaacVector> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            /*opt<SizeT>*/ auto sizeOpt = SizeT::parse(readPtr);
            if (!sizeOpt) [[unlikely]]
                return std::nullopt;
            const auto size = static_cast<size_t>(*sizeOpt);

            JaacVector self{};
            self.reserve(size);

            for (size_t i = 0; i < size; ++i) {
                /*opt<ElemT>*/ auto j = ElemT::parse(readPtr);
                if (!j) [[unlikely]]
                    return std::nullopt;
                self.emplace_back(std::move(*j));
            }

            return std::optional{self};
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR> && JaacParsable<ElemT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            /*opt<SizeT>*/ auto sizeOpt = SizeT::parse(readPtr);
            if (!sizeOpt) [[unlikely]]
                return false;
            const auto size = static_cast<size_t>(*sizeOpt);

            auto first = readPtr.i;

            for (size_t i = 0; i < size; ++i) {
                if (!ElemT::check_and_move(readPtr)) [[unlikely]]
                    return false;
            }

            return true;
        }

        template <typename SpecW, typename Self>
            requires JaacDeserable<SizeT, SpecW> && JaacDeserable<ElemT, SpecW> &&
                     (settings.isIndepPref || JaacInCollectionDeserable<ElemT, std::remove_cvref_t<Self>, SpecW>)
        bool deser(this Self&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            if (!static_cast<SizeT>(self.size()).deser(writePtr)) [[unlikely]]
                return false;

            for (auto& i : self) {
                if (!i.deser(writePtr)) [[unlikely]]
                    return false;
            }

            return true;
        }

        template <typename SpecW, typename Self>
            requires JaacDeserable<SizeT, SpecW> &&
                     JaacInCollectionDeserable<ElemT, std::remove_cvref_t<Self>, SpecW> &&
                     (!settings.isIndepPref || JaacInCollectionDeserable<ElemT, std::remove_cvref_t<Self>, SpecW>)
        bool deser(this Self&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            auto size = self.size();

            if (!static_cast<SizeT>(size).deser(writePtr)) [[unlikely]]
                return false;

            for (size_t i = 0; i < size; ++i) {
                if (!self[i].deser(writePtr, i, self)) [[unlikely]]
                    return false;
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

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            /*opt<SizeT>*/ auto sizeOpt = SizeT::parse(readPtr);
            if (!sizeOpt) [[unlikely]]
                return false;

            if (!readPtr.check_and_move(sizeof(RawElemT) * static_cast<size_t>(*sizeOpt))) [[unlikely]]
                return false;

            return true;
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static std::optional<RawJaacSpan> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            auto sizePtr = readPtr.i;
            if (!check_and_move(readPtr)) [[unlikely]]
                return std::nullopt;
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
            /*opt<SizeT>*/ auto sizeOpt = SizeT::parse(readPtr);
            if (!sizeOpt) [[unlikely]]
                return std::nullopt;
            SizeT typedSize = *sizeOpt;
            const auto size = static_cast<size_t>(typedSize);

            u8_ptr first = readPtr.i;
            if (!readPtr.check_and_move(sizeof(RawElemT) * size)) [[unlikely]]
                return std::nullopt;

            return std::optional{RawFullCachedJaacSpan{first, readPtr.i, sizePtr, typedSize}};
        }

        template <typename SpecR>
            requires JaacParsable<SizeT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            /*opt<SizeT>*/ auto sizeOpt = SizeT::parse(readPtr);
            if (!sizeOpt) [[unlikely]]
                return false;

            if (!readPtr.check_and_move(sizeof(RawElemT) * static_cast<size_t>(*sizeOpt))) [[unlikely]]
                return false;

            return true;
        }

        template <typename SpecW>
            requires JaacDeserable<SizeT, SpecW>
        bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            if (!self.typedSize.deser(writePtr)) [[unlikely]]
                return false;
            writePtr.deser_range(self);
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

        /*implicit*/ operator RawJaacSpan<RawElemT, SizeT>() noexcept {
            return RawJaacSpan<RawElemT, SizeT>{sizePtr};
        }
    };

    namespace internal {
        template <typename AttrsSpan>
        struct FullAttributeMethodsPart {
            template <typename SpecW>
                requires JaacDeserable<AttrsSpan, SpecW>
            bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                return self.attrNameIdx.deser(writePtr) && static_cast<AttrsSpan>(self).deser(writePtr);
            }
        };
    } // namespace internal

    template <typename AttrsSpan = RawJaacSpan<uint8_t, wrappers::AttrsInnerSizeT>>
    struct FullAttribute;

    using DefVeryFullAttribute = FullAttribute<RawFullCachedJaacSpan<uint8_t, wrappers::AttrsInnerSizeT>>;

    template <typename AttrsSpan = RawJaacSpan<uint8_t, wrappers::AttrsInnerSizeT>>
    struct FullAttribute : AttrsSpan,
                           SerCombinatorPMxn<FullAttribute<AttrsSpan>, wrappers::CpoolSizeT, AttrsSpan>,
                           internal::FullAttributeMethodsPart<AttrsSpan> {
        using AttrsSpan::AttrsSpan;

        wrappers::CpoolSizeT attrNameIdx{};

        constexpr FullAttribute(const wrappers::CpoolSizeT name, AttrsSpan& span) noexcept :
            AttrsSpan(span), attrNameIdx(name) {
        }

        constexpr FullAttribute(const wrappers::CpoolSizeT name, AttrsSpan&& span) noexcept :
            AttrsSpan(std::move(span)), attrNameIdx(name) {
        }

        /*implicit*/ constexpr FullAttribute(DefVeryFullAttribute& veryFull)
            requires(!std::same_as<FullAttribute, DefVeryFullAttribute>)
            : AttrsSpan(veryFull), attrNameIdx(veryFull.attrNameIdx) {
        }
        /*implicit*/ constexpr FullAttribute(DefVeryFullAttribute&& veryFull)
            requires(!std::same_as<FullAttribute, DefVeryFullAttribute>)
            : AttrsSpan(veryFull), attrNameIdx(veryFull.attrNameIdx) {
        }

    private:
        using Base3 = internal::FullAttributeMethodsPart<AttrsSpan>;

    public:
        using Base3::deser;
        using SerCombinatorPMxn<FullAttribute, wrappers::CpoolSizeT, AttrsSpan>::parse;
        using SerCombinatorPMxn<FullAttribute, wrappers::CpoolSizeT, AttrsSpan>::check_and_move;
    };

    using DefAttrsCont = JaacVector<FullAttribute<>>;

    template <typename AttrsCont = DefAttrsCont>
    struct AttrsComponent : SerCombinatorPMxn<AttrsComponent<AttrsCont>, AttrsCont> {
        AttrsCont attrs{};

        explicit constexpr AttrsComponent(AttrsCont& attrs) noexcept : attrs(attrs) {
        }

        explicit constexpr AttrsComponent(AttrsCont&& attrs) noexcept : attrs(std::move(attrs)) {
        }

        constexpr AttrsComponent() noexcept = default;

        template <typename SpecW>
            requires JaacDeserable<AttrsCont, SpecW>
        bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            return self.attrs.deser(writePtr);
        }

        using SerCombinatorPMxn<AttrsComponent, AttrsCont>::parse;
        using SerCombinatorPMxn<AttrsComponent, AttrsCont>::check_and_move;
    };

    template <typename Base, typename AttrsCont = DefAttrsCont>
    using HasAttrWMxn = AddPostfieldsWMxn<Base, AttrsComponent<AttrsCont>>;

    namespace internal {
        struct MFInfoNoattrDeserPart {
            template <typename SpecW>
            bool deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                return self.accessFlags.deser(writePtr) && self.nameIdx.deser(writePtr) &&
                       self.descriptorIdx.deser(writePtr);
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
    using MFInfo = HasAttrWMxn<MFInfoNoattr, AttrsCont>;
} // namespace parser::common

export namespace parser::wrappers {
    using SVersionNumber = Number<uint16_t>;

    struct VersionNumber : common::SerCombinatorPMxn<VersionNumber, SVersionNumber, SVersionNumber> {
        SVersionNumber minor{}, major{};

        constexpr VersionNumber(const SVersionNumber minor, const SVersionNumber major) noexcept :
            minor(minor), major(major) {
        }

        constexpr VersionNumber() noexcept = default;

        template <typename SpecW>
        bool deser(WritePtrBySpec<SpecW>& writePtr) const noexcept {
            return minor.deser(writePtr) && major.deser(writePtr);
        }
    };
} // namespace parser::wrappers
