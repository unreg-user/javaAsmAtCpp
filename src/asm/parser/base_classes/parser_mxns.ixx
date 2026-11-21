module;

#include <optional>
#include <tuple>

#include "../../../cstd/macro.h"

export module parser_mxns;

import parser_utils;
import simple_cstd;
import gstd;
import parser_concepts;
import macro_u;

export namespace parser::mxns {
    struct JaacIterablePMxn {
        decltype(auto) jaac_begin(this auto&& self) {
            return self.begin();
        }

        decltype(auto) jaac_end(this auto&& self) {
            return self.end();
        }
    };

    struct JaacSpanPMxn : JaacIterablePMxn {
        decltype(auto) jaac_size(this auto&& self) {
            return self.size();
        }
    };

    struct JaacVectorPMxn : JaacSpanPMxn {
        void jaac_reserve(this auto&& self, size_t i) {
            self.reserve(i);
        }

        template <typename Element>
        gstd::true_void jaac_push_back(this auto&& self, Element&& element)
            requires requires { self.push_back(std::forward<Element>(element)); }
        {
            self.push_back(std::forward<Element>(element));
            return {};
        }

        template <typename Element>
        gstd::true_void jaac_emplace_back(this auto&& self, Element&& element)
            requires requires { self.emplace_back(std::forward<Element>(element)); }
        {
            self.emplace_back(std::forward<Element>(element));
            return {};
        }
    };

    struct HasParseSizePMxn {
        size_t parseSize = 0;

        NODISCARD size_t jaac_get_parse_size() const noexcept {
            return parseSize;
        }

        void jaac_set_parse_size(const size_t size) noexcept {
            parseSize = size;
        }
    };

    struct Translator {
    protected:
        u8_ptr sourceBegin = nullptr;
        u8_ptr sourceEnd = nullptr;
        size_t sourceSize = 0;

    public:
        constexpr Translator() noexcept = default;

        constexpr void jaac_set_source_data_interval_begin(uint8_t* pos) noexcept {
            sourceBegin = pos;
        }

        constexpr void jaac_set_source_data_interval_end(uint8_t* pos) noexcept {
            sourceEnd = pos;
        }

        constexpr void jaac_set_source_data_size(const size_t iSize) noexcept {
            sourceSize = iSize;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_interval_begin(this auto&& self) noexcept {
            return self.sourceBegin;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_interval_end(this auto&& self) noexcept {
            return self.sourceEnd;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_size(this auto&& self) noexcept {
            return self.sourceSize;
        }
    };

    struct PostcompleteIsDeepParsePMxn {
        template <typename ClassNT>
        bool jaac_postcomplete_handle(this auto&& self, ClassNT& classNode) {
            return self.deep_parse(classNode);
        }
    };
} // namespace parser::mxns

// b2
export namespace parser::mxns {
    template <typename Base, typename Self>
    struct RewrapperPMxn {
    private:
        NODISCARD static std::optional<Self> opt_impl_construct_self(auto&& from) noexcept {
            ul_assert_nullopt(from);
            return std::optional{Self(std::forward_like<decltype(from)>(*from))};
        }

    public:
        template <typename SpecR>
        NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return opt_impl_construct_self(Base::parse(readPtr));
        }

        template <typename SpecR>
        NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return opt_impl_construct_self(Base::parse(std::move(readPtr)));
        }
    };

    template <typename WidType, typename Self>
    struct JaacWidstruct : WidType, RewrapperPMxn<WidType, Self> {
        using WidType::WidType;

    private:
        using RewrBase1 = RewrapperPMxn<WidType, Self>;

    protected:
        IMPLICIT constexpr JaacWidstruct(WidType& widType) noexcept : WidType(widType) {
        }
        IMPLICIT constexpr JaacWidstruct(WidType&& widType) noexcept : WidType(std::move(widType)) {
        }

    public:
        using RewrBase1::parse;
    };

    template <typename Self, typename... Types>
    struct SerCombinatorPMxn {
    private:
        template <typename... Types2>
        NODISCARD static std::optional<Self> after_read_parse(std::tuple<Types2...>&& parsedTuple) noexcept {
            ul_assert_nullopt(std::apply(
                    // "keyword"
                    []<typename... Parsed>(Parsed&&... parsed) {
                        return (... && make_assertor(std::forward<Parsed>(parsed)));
                    },
                    parsedTuple));

            return std::apply(
                    []<typename... Parsed>(Parsed&&... parsed) -> std::optional<Self> {
                        return std::optional<Self>{Self{(*std::forward<Parsed>(parsed))...}};
                    },
                    std::move(parsedTuple));
        }

    public:
        template <typename SpecR>
            requires(... && JaacParsable<Types, SpecR>)
        NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return after_read_parse(std::tuple{Types::parse(readPtr)...});
        }

        template <typename SpecR>
            requires(... && JaacParsable<Types, SpecR>)
        NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return [&]<size_t... Is>(std::index_sequence<Is...>) -> std::optional<Self> {
                static constexpr size_t Last = sizeof...(Types) - 1;
                return after_read_parse(
                        std::tuple{Types...[Is] ::parse(readPtr)..., Types...[Last] ::parse(std::move(readPtr))});
            }(std::make_index_sequence<sizeof...(Types) - 1>{});
        }

        template <typename SpecR>
            requires(... && JaacParsable<Types, SpecR>)
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return (... && make_assertor(Types::check_and_move(readPtr)));
        }

        template <typename SpecR>
            requires(... && JaacParsable<Types, SpecR>)
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return [&]<size_t... Is>(std::index_sequence<Is...>) -> bool {
                static constexpr size_t Last = sizeof...(Types) - 1;
                return (... && make_assertor(Types...[Is] ::check_and_move(readPtr))) &&
                       make_assertor(Types...[Last] ::check_and_move(std::move(readPtr)));
            }(std::make_index_sequence<sizeof...(Types) - 1>{});
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
                ul_assert_nullopt(prefSt && base);
                return std::optional{Self{*std::move(prefSt), *std::move(base)}};
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PrefieldsSt, SpecR>
            NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                auto prefSt = PrefieldsSt::parse(readPtr);
                auto base = Base::parse(std::move(readPtr));
                ul_assert_nullopt(prefSt && base);
                return std::optional{Self{*std::move(prefSt), *std::move(base)}};
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PrefieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
                ul_assert_false(PrefieldsSt::check_and_move(readPtr));
                ul_assert_false(Base::check_and_move(readPtr));
                return true;
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PrefieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                ul_assert_false(PrefieldsSt::check_and_move(readPtr));
                ul_assert_false(Base::check_and_move(std::move(readPtr)));
                return true;
            }

            template <typename SpecW, typename DirSelf>
                requires JaacDeserable<Base, SpecW> && JaacDeserable<PrefieldsSt, SpecW>
            bool deser(this DirSelf&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                ul_assert_false(cstd::forward_static_cast<PrefieldsSt>(std::forward<DirSelf>(self)).deser(writePtr));
                ul_assert_false(cstd::forward_static_cast<Base>(std::forward<DirSelf>(self)).deser(writePtr));
                return true;
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

        constexpr AddPrefieldsWMxn(PrefieldsSt&& prefieldsStruct, Base&& base) noexcept :
            Base2(std::move(prefieldsStruct)), Base(std::move(base)) {
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
                ul_assert_nullopt(base && postSt);
                return std::optional{Self{*std::move(base), *std::move(postSt)}};
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PostfieldsSt, SpecR>
            NODISCARD static std::optional<Self> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                /*opt<Base>*/ auto base = Base::parse(readPtr);
                /*opt<PostfieldsSt>*/ auto postSt = PostfieldsSt::parse(std::move(readPtr));
                ul_assert_nullopt(base && postSt);
                return std::optional{Self{*std::move(base), *std::move(postSt)}};
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PostfieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
                ul_assert_false(Base::check_and_move(readPtr));
                ul_assert_false(PostfieldsSt::check_and_move(readPtr));
                return true;
            }

            template <typename SpecR>
                requires JaacParsable<Base, SpecR> && JaacParsable<PostfieldsSt, SpecR>
            NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
                ul_assert_false(Base::check_and_move(readPtr));
                ul_assert_false(PostfieldsSt::check_and_move(std::move(readPtr)));
                return true;
            }

            template <typename SpecW, typename DirSelf>
                requires JaacDeserable<Base, SpecW> && JaacDeserable<PostfieldsSt, SpecW>
            bool deser(this DirSelf&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
                ul_assert_false(cstd::forward_static_cast<Base>(std::forward<DirSelf>(self)).deser(writePtr));
                ul_assert_false(cstd::forward_static_cast<PostfieldsSt>(std::forward<DirSelf>(self)).deser(writePtr));
                return true;
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

        constexpr AddPostfieldsWMxn(Base&& base, PostfieldsSt&& prefieldsStruct) noexcept :
            Base(std::move(base)), Base2(std::move(prefieldsStruct)), Base3() {
        }

    public:
        using Base::Base;

        using Base3::check_and_move;
        using Base3::deser;
        using Base3::parse;
    };
} // namespace parser::mxns
