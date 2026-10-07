module;

#include <span>
#include <string>
#include <vector>
#include "../../../cstd/macro.h"

export module nodes;

import const_i;
import parser_utils;
import array_parsers;

export namespace nodes {
    namespace templ {
        struct ParseFlags {
            bool useWrite;
            bool handleVersion = false;
            parser::HandleMode handleWrittenCpool, handleWrittenIfaces, handleWrittenField = parser::HandleMode::NONE;
        };

        template <typename Base>
        struct JaacCollectionMxn : Base {
            using Base::T;

            void jaac_reserve(this auto&& self, size_t i) {
                self.reserve(i);
            }

            bool jaac_push_back(this auto&& self, auto&& element) {
                self.push_back(std::forward<decltype(element)>(element));
                return true;
            }

            decltype(auto) jaac_at(this auto&& self, size_t i) {
                return self.at(i);
            }

            auto jaac_begin(this auto&& self) {
                return self.begin();
            }

            auto jaac_end(this auto&& self) {
                return self.end();
            }

            auto jaac_size(this auto&& self) {
                return self.size();
            }
        };

        namespace def {
            template <typename Base>
            struct CpoolMxn : Base {
                using Base::T;
                cp_consts::SizeT parse_size = 0;

                NODISCARD size_t jaac_get_parse_size() const {
                    return parse_size;
                }

                void jaac_set_parse_size(const size_t size) {
                    parse_size = size;
                }
            };

            template <typename IBase>
            using DefCpoolCollMxn = CpoolMxn<JaacCollectionMxn<IBase>>;

            struct CpoolRawColl : DefCpoolCollMxn<std::span<uint8_t>> {
                using DefCpoolCollMxn<std::span<uint8_t>>::DefCpoolCollMxn;

                template <typename SpecR>
                bool jaac_push_back(this auto&& self, parser::FileReadPtrBySpec<SpecR> element) {
                    self.push_back(element.readSpan());
                    return true;
                }
            };

            struct CpoolParsedColl : DefCpoolCollMxn<std::vector<cp_consts::traits::variant>> {
                using DefCpoolCollMxn<std::vector<cp_consts::traits::variant>>::DefCpoolCollMxn;

                template <typename SpecR>
                bool jaac_push_back(this auto&& self, parser::FileReadPtrBySpec<SpecR> element) {
                    cp_consts::traits::variant var{};
                    if (parser::parse1_const<true>(element, [&](const cp_consts::traits::variant& v){ var = v; }) != parser::ParseErr::NONE) [[unlikely]] {
                        return false;
                    }
                    self.push_back(var);
                }
            };
        } // namespace def
    } // namespace cmp

    namespace full {
        template <templ::ParseFlags parseFlagsG>
        struct ClassN {
            PUBLIC_V_GENERIC_R(parseFlags)

            uint32_t version{};
            templ::Translator cpool, ifaces, fields;
        };
    } // namespace full
} // namespace nodes
