module;

#include <variant>

#include "../../../cstd/macro.h"

export module nodes;

import cp_consts;
import parser_utils;
import array_parsers;
import parser_b2;
import parser_b1;
import simple_cstd;
import parser_utils;
import gstd;

export namespace nodes {
    namespace templ {
        struct ParseFlags { // NOLINT(*-pro-type-member-init)
            bool useWrite;
            bool handleVersion, handleAccess, deepHandleThisClassIdx, handleSuperClassIdx = false;
            bool parseCpoolStructure = false;
            parser::HandleMode handleModeCpool, handleModeIfaces, handleModeFields, handleModeMethods, handleModeAttrs = parser::HandleMode::NONE;
            parser::ParseLength parseLength = parser::ParseLength::FULL;
        };

        template <ParseFlags parseFlags>
        struct ClearClassN {
            PUBLIC_V_GENERIC_R_AT(parseFlags, useWrite)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleVersion)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleAccess)
            PUBLIC_V_GENERIC_R_AT(parseFlags, deepHandleThisClassIdx)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleSuperClassIdx)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleModeCpool)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleModeIfaces)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleModeFields)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleModeMethods)
            PUBLIC_V_GENERIC_R_AT(parseFlags, handleModeAttrs)
            PUBLIC_V_GENERIC_R_AT(parseFlags, parseCpoolStructure)
            PUBLIC_V_GENERIC_R_AT(parseFlags, parseLength)
        };

        namespace def {
            using namespace parser;
            using namespace common;
            using namespace cp_consts::traits;

            struct JaacCpoolCollectionPMxn : JaacVectorPMxn, HasParseSizePMxn {};

            template <typename Base>
            using MakeHasParseSize = cstd::ApplyP<Base, HasParseSizePMxn>;

            using CpoolRawColl = JaacVector<RawJaacSpan<>>;

            using CpoolParsedColl = JaacVector<cp_var>;

            struct CpoolAndRawVar : std::variant<cp_var, RawAnyJaacSpan<>> {
            private:
                using Base = std::variant<cp_var, RawAnyJaacSpan<>>;

            public:
                /*implicit*/ constexpr CpoolAndRawVar(cp_var var) noexcept : Base(var) {}
                /*implicit*/ constexpr CpoolAndRawVar(RawAnyJaacSpan<> span) noexcept : Base(span) {}

                template <typename SpecW>
                NODISCARD gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) noexcept {
                    std::visit([&writePtr](auto self) {
                        self.deser(writePtr);
                    }, *this);
                    return {};
                }
            };

            using CpoolHalfParsedColl = JaacVector<CpoolAndRawVar>;
        } // namespace def
    } // namespace templ

    namespace full {
        template <templ::ParseFlags parseFlags>
        struct EmptyClassN : templ::ClearClassN<parseFlags> {
            parser::common::Translator cpool, ifaces, fields, methods, attrs;
        };
    } // namespace full
} // namespace nodes
