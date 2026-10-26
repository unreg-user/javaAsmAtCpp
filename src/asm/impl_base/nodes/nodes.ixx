module;

#include <span>
#include <string>
#include <vector>
#include "../../../cstd/macro.h"

export module nodes;

import cp_consts;
import parser_utils;
import array_parsers;
import parser_b2;
import parser_b1;

export namespace nodes {
    namespace templ {
        struct ParseFlags {
            bool useWrite;
            bool handleVersion = false;
            parser::HandleMode handleWrittenCpool, handleWrittenIfaces, handleWrittenField = parser::HandleMode::NONE;
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
            using DefCpoolCollMxn = CpoolMxn<parser::common::JaacCollectionMxn<IBase>>;

            struct CpoolRawColl : DefCpoolCollMxn<std::span<uint8_t>> {
                using DefCpoolCollMxn<std::span<uint8_t>>::DefCpoolCollMxn;

                template <typename SpecR>
                bool jaac_push_back(this auto&& self, parser::ReadPtrBySpec<SpecR> element) {
                    self.push_back(element.readSpan());
                    return true;
                }
            };

            struct CpoolParsedColl : DefCpoolCollMxn<std::vector<cp_consts::traits::variant>> {
                using DefCpoolCollMxn<std::vector<cp_consts::traits::variant>>::DefCpoolCollMxn;

                template <typename SpecR>
                bool jaac_push_back(this auto&& self, parser::ReadPtrBySpec<SpecR> element) {
                    cp_consts::traits::variant var{};
                    if (parser::parse1_const<true>(element, [&](const cp_consts::traits::variant& v) { var = v; }) !=
                        parser::ParseErr::NONE) [[unlikely]] {
                        return false;
                    }
                    self.push_back(var);
                    return true;
                }
            };
        } // namespace def
    } // namespace templ

    namespace full {
        template <templ::ParseFlags parseFlagsG>
        struct ClassN {
            PUBLIC_V_GENERIC_R(parseFlags)

            uint32_t version{};
            parser::common::Translator cpool, ifaces, fields;
        };
    } // namespace full
} // namespace nodes
