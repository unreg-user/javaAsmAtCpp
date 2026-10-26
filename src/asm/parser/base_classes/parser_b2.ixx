module;

#include <optional>
#include <span>
#include <tuple>
#include "../../../cstd/macro.h"

export module parser_b2;

import array_parsers;
import parser_b1;
import parser_concepts;
import read_utils;
import parser_utils;
import simple_cstd;
import parser_wrappers_b2;

export namespace parser::common {
    namespace internal_bases {
        template <typename Base, typename... Types>
        struct SerCombinatorMxnForMxn : Base {
            using Base::Base;

            template <typename SpecR>
                requires(... && JaacParsable<Types, SpecR>)
            NODISCARD static std::optional<SerCombinatorMxnForMxn> parse(ReadPtrBySpec<SpecR>& readPtr) {
                std::tuple<std::optional<Types>...> parsedTuple{Types::template parse(readPtr)...};
                return std::apply([](auto&&... parsed) { return (... && parsed); }, parsedTuple)
                               ? std::apply(
                                         [](auto&&... parsed) {
                                             return std::make_optional(SerCombinatorMxnForMxn{(*parsed)...});
                                         },
                                         std::move(parsedTuple))
                               : std::nullopt;
            }

            template <typename SpecR>
                requires(... && JaacParsable<Types, SpecR>)
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) {
                return (... && Types::template check_and_move(readPtr));
            }
        };

        template <typename Base, typename PrefieldsSt>
        struct AddPrefieldsMxn : PrefieldsSt, Base {
        private:
            using Base2 = PrefieldsSt;

            AddPrefieldsMxn(PrefieldsSt prefieldsStruct, Base base) : Base2(prefieldsStruct), Base(base) {
            }

        public:
            using Base::Base;

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PrefieldsSt, SpecR>
            NODISCARD static std::optional<AddPrefieldsMxn> parse(ReadPtrBySpec<SpecR>& readPtr) {
                std::optional<PrefieldsSt> prefSt = PrefieldsSt::parse(readPtr);
                std::optional<Base> base = Base::parse(readPtr);
                return prefSt && base ? std::make_optional(AddPrefieldsMxn{*prefSt, *base}) : std::nullopt;
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PrefieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) {
                return PrefieldsSt::check_and_move(readPtr) && Base::check_and_move(readPtr);
            }

            template <typename SpecW>
                requires JaacDeserable<Base, SpecW> && JaacDeserable<PrefieldsSt, SpecW>
            NODISCARD bool deser(WritePtrBySpec<SpecW>& writePtr) {
                return static_cast<PrefieldsSt>(*this).deser(writePtr) && static_cast<Base>(*this).deser(writePtr);
            }
        };

        template <typename Base, typename PostfieldsSt>
        struct AddPostfieldsMxn : Base, PostfieldsSt {
        private:
            using Base2 = PostfieldsSt;

            AddPostfieldsMxn(Base base, PostfieldsSt prefieldsStruct) : Base(base), Base2(prefieldsStruct) {
            }

        public:
            using Base::Base;

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PostfieldsSt, SpecR>
            NODISCARD static std::optional<AddPostfieldsMxn> parse(ReadPtrBySpec<SpecR>& readPtr) {
                std::optional<Base> base = Base::parse(readPtr);
                std::optional<PostfieldsSt> postSt = PostfieldsSt::parse(readPtr);
                return base && postSt ? std::make_optional(AddPostfieldsMxn{*base, *postSt}) : std::nullopt;
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PostfieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) {
                return Base::check_and_move(readPtr) && PostfieldsSt::check_and_move(readPtr);
            }

            template <typename SpecW>
                requires JaacDeserable<Base, SpecW> && JaacDeserable<PostfieldsSt, SpecW>
            NODISCARD bool deser(WritePtrBySpec<SpecW>& writePtr) {
                return static_cast<Base>(*this).deser(writePtr) && static_cast<PostfieldsSt>(*this).deser(writePtr);
            }
        };
    } // namespace internal_bases

    template <typename ElType, typename SizeT = ArraySizeT>
    struct JaacSpan : std::span<ElType> {
        using std::span<ElType>::span;

        template <typename SpecW>
            requires JaacDeserable<ElType, SpecW> && JaacDeserable<ElType, SizeT>
        NODISCARD bool deser(WritePtrBySpec<SpecW>& writePtr) {
            if (!static_cast<SizeT>(this->size()).deser(writePtr))
                return false;
            for (auto& i : *this)
                if (!i.deser(writePtr))
                    return false;
            return true;
        }
    };

    template <typename RawElType, typename SizeT = ArraySizeT>
    struct RawJaacSpan : std::span<RawElType> {
        using std::span<RawElType>::span;

        template <typename SpecR>
            requires JaacParsable<ArraySizeT, SpecR>
        NODISCARD static std::optional<RawJaacSpan> parse(ReadPtrBySpec<SpecR>& readPtr) {
            std::optional<SizeT> sizeOpt = SizeT::template parse(readPtr);
            if (!sizeOpt)
                return std::nullopt;
            const auto size = static_cast<size_t>(*sizeOpt);

            auto first = readPtr.i;
            if (!readPtr.check_and_move(sizeof(RawElType) * size))
                return std::nullopt;

            return std::make_optional(RawJaacSpan{first, size});
        }

        template <typename SpecR>
            requires JaacParsable<ArraySizeT, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) {
            std::optional<SizeT> sizeOpt = SizeT::template parse(readPtr);
            if (!sizeOpt)
                return false;
            const auto size = static_cast<size_t>(*sizeOpt);

            auto first = readPtr.i;
            if (!readPtr.check_and_move(sizeof(RawElType) * size))
                return false;

            return true;
        }

        template <typename SpecW>
            requires JaacDeserable<ArraySizeT, SpecW>
        NODISCARD bool deser(WritePtrBySpec<SpecW>& writePtr) {
            if (!static_cast<SizeT>(this->size()).deser(writePtr))
                return false;
            writePtr.deser_range(*this);
            return true;
        }
    };

    namespace aliases {
        template <K2 Cont>
        using ApplyAttrsGsToSpan = Cont</*el*/ uint8_t, /*size_t*/ ArraySizeT>;

        using RawAttrsSpan = ApplyAttrsGsToSpan<RawJaacSpan>;

        template <KN Container, typename... Types>
        using ApplyCombineSerrMxnForBy = internal_bases::SerCombinatorMxnForMxn<Container<Types...>, Types...>;
    } // namespace aliases

    namespace internal_bases {
        template <typename AttrsSpan>
        struct AttrsCont0 : AttrsSpan {
            using AttrsSpan::AttrsSpan;

        protected:
            explicit AttrsCont0(AttrsSpan& span) : AttrsSpan(span) {}
            explicit AttrsCont0(AttrsSpan&& span) : AttrsSpan(std::move(span)) {}
            explicit AttrsCont0(AttrsSpan span) : AttrsSpan(span) {}
        public:

            wrappers::CpoolIdx attr_name_idx{};

            template <typename SpecW>
                requires JaacDeserable<AttrsSpan, SpecW>
            NODISCARD bool deser(WritePtrBySpec<SpecW>& writePtr) {
                return attr_name_idx.deser(writePtr) && static_cast<AttrsSpan>(*this).deser(writePtr);
            }
        };

        template <typename AttrsCont>
        struct AttrsComponent0 {
            AttrsCont attrs;

            template <typename SpecW>
                requires JaacDeserable<AttrsCont, SpecW>
            NODISCARD bool deser(WritePtrBySpec<SpecW>& writePtr) {
                return attrs.deser(writePtr);
            }
        };
    } // namespace internal_bases

    namespace aliases {
        template <typename AttrsSpan = RawAttrsSpan>
        using AttrsCont = ApplyCombineSerrMxnForBy<internal_bases::AttrsCont0, AttrsSpan>;

        template <typename AttrsCont = AttrsCont>
        using AttrsComponent = internal_bases::SerCombinatorMxnForMxn<internal_bases::AttrsComponent0<AttrsCont>, AttrsCont>;

        template <typename Base, typename AttrsComp = AttrsComponent>
        using HasAttrInfoMxn = internal_bases::AddPostfieldsMxn<Base, AttrsComp>;
    } // namespace aliases

    namespace internal_bases {
        struct FieldInfo0 { // NOLINT(*-pro-type-member-init)
            wrappers::AccessFlags access_flags;
            wrappers::CpoolIdx name_idx, descriptor_idx;
            
            template <typename SpecW>
            NODISCARD bool deser(WritePtrBySpec<SpecW>& writePtr) {
                return access_flags.deser(writePtr) && name_idx.deser(writePtr) && descriptor_idx.deser(writePtr);
            }
        };
        
        using FieldInfo0FullNoattr = SerCombinatorMxnForMxn<FieldInfo0, wrappers::AccessFlags, wrappers::CpoolIdx, wrappers::CpoolIdx>;
    }
    
    namespace aliases {
        template <typename AttrsComp = AttrsComponent>
        using FieldInfo = HasAttrInfoMxn<internal_bases::FieldInfo0FullNoattr, AttrsComp>;
    }
} // namespace parser::common
