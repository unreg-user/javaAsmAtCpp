
#define MAIN_IS_EXP 0


#include <chrono>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>
#include "asm/impl_base/nodes/nodes_macro.h"
#include "asm/parser/parser_mcr.h"
#include "cstd//macro.h"

#if !MAIN_IS_EXP

import parser;
import nodes;
import reader;
import simple_cstd;
import parser_mxns;
import parser_base;
import parser_widstructs;
import parser_utils;
import cp_consts;
import costd;
import compl_flags;
import parser_concepts;
import gstd;
import code_attr;
import array_parsers;
import cstd_variant;

using namespace parser;
using namespace mxns;
using namespace common;
using namespace wrappers;
using namespace nodes::templ;
using namespace cp_consts::traits;


namespace {
    struct Cpool : JaacWidstruct<cp_consts::CpoolJaacVector<def::CpoolAndRawVar, {{}, true}>, Cpool>, HasParseSizePMxn {
        using JaacWidstruct::JaacWidstruct;
        friend RewrapperPMxn;
    };

    template <template <typename...> typename VarT, typename RawAttr, typename... ParsedAttrs>
    struct PartialAttribute : VarT<RawAttr, ParsedAttrs...> {
        using VarT<RawAttr, ParsedAttrs...>::VarT;

    private:
        using Base = VarT<RawAttr, ParsedAttrs...>;

    public:
        template <typename ClassNT, typename... Args>
            requires JaacClassNT<ClassNT>
        bool deep_parse(ClassNT& classNode, Args&&... args) {
            auto rawAttr = this->template get<0>();
            UncheckedReadPtr attrInterval =
                    static_cast<RawJaacSpan<uint8_t, AttrsInnerSizeT>>(rawAttr).get_data_interval();
            auto constant = classNode.cpool[gstd::unwrap(rawAttr.get_attr_name_idx())];
            int d = 0;
            return true;
        }

        template <typename SpecR>
            requires JaacParsable<RawAttr, SpecR>
        NODISCARD static std::optional<PartialAttribute> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            auto opt = RawAttr::parse(readPtr);
            if (!opt) [[unlikely]] {
                return std::nullopt;
            }
            return PartialAttribute(std::move(*opt));
        }

        template <typename SpecR>
            requires JaacParsable<RawAttr, SpecR>
        NODISCARD static std::optional<PartialAttribute> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            auto opt = RawAttr::parse(std::move(readPtr));
            if (!opt) [[unlikely]] {
                return std::nullopt;
            }
            return PartialAttribute(std::move(*opt));
        }

        template <typename SpecR>
            requires JaacParsable<RawAttr, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return RawAttr::check_and_move(readPtr);
        }

        template <typename SpecR>
            requires JaacParsable<RawAttr, SpecR>
        NODISCARD static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return RawAttr::check_and_move(std::move(readPtr));
        }

        template <typename SpecW>
            requires(JaacDeserable<RawAttr, SpecW> && (... && JaacDeserable<ParsedAttrs, SpecW>) )
        NODISCARD decltype(auto) deser(this auto&& self, WritePtrBySpec<SpecW>& writePtr) noexcept {
            return std::forward_like<decltype(self)>(self).visit(
                    [&writePtr]<typename Slf>(Slf&& slf) { return std::forward<Slf>(slf).deser(writePtr); });
        }
    };

    template <typename... ParsedAttrs>
    using DefPartialAttribute =
            PartialAttribute<cstd::variant_of_ptrs_and_little_t_ptr, MinAttribute<>, ParsedAttrs...>;

    struct Methods : JaacWidstruct<JaacVector<MFInfo<JaacVector</*attr*/ DefPartialAttribute<>>>>, Methods>,
                     PostcompleteIsDeepParsePMxn {
        using JaacWidstruct::JaacWidstruct;
        friend RewrapperPMxn;
    };

    struct ClassN1 : ClearClassN<ParseFlags{.parseCpoolStructure = true,
                                            .handleModeCpool = HandleMode::FULL,
                                            .handleModeIfaces = HandleMode::FULL,
                                            .handleModeFields = HandleMode::FULL,
                                            .handleModeMethods = HandleMode::FULL,
                                            .handleModeAttrs = HandleMode::FULL,
                                            .parseLength = ParseLength::FULL}> {
        Cpool cpool{};
        JaacVector<CpoolSizeT> ifaces{};
        JaacVector<MFInfo<>> fields{};
        Methods methods{};
        JaacVector<FullAttribute<>> attrs{};
    };

    using InData = std::span<uint8_t>;
    using OutData = std::vector<uint8_t>;
} // namespace

int main() {
    // auto readPtr = ReadPtrBySpec<cflags::class_file_ptr>{nullptr, nullptr};
    // std::vector<uint8_t> musor;
    // auto writePtr = WritePtrBySpec<cflags::class_file_ptr>{musor};

    const auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < 1; ++i) {
        auto path = reader::PreRWFileData{"D:/you_p_only/cpp_elems/javaAsmAtCpp/test/test.class"};
        const auto opened = path.open("rb");
        assert(opened);
        OutData output{};
        auto read = opened->read_file();

        DISCARD opened->close();
        if (auto result = parser::unsafe_parse<ClassN1>(read, &output)) {
            auto [node, flag] = result->to_pair();
            int d = 0;
            // ReadPtrBySpec<cflags::class_file_ptr> r{nullptr, nullptr};
        } else {
            std::cerr << "Test failed: " << static_cast<uint32_t>(result.error().at) << std::endl;
        }
        read.unchecked_close();
    }
    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << end - start;

    // auto result6 = parser::parse<nodes::full::EmptyClassN<{.useWrite = true}>>(path);
}

#else

namespace {
    struct U16Wrapper {
        uint16_t g;

        IMPLICIT constexpr U16Wrapper(const uint16_t g) noexcept : g{g} {
        }
    };

    template <typename Base>
    struct AnyWMxn : Base {
        using Base::Base;

        using Base2 = Base;

        std::string_view g = "LLLLLLL";

        decltype(auto) d(this auto&& self) {
            return self.Base2::g;
        }
    };

    struct Any : U16Wrapper {
        using Base = U16Wrapper;
        using Base::Base;

        std::string_view g = "LLLLLLL";

        decltype(auto) d(this auto&& self) {
            return self.Base::g;
        }
    };
} // namespace

int main() {
    AnyWMxn<U16Wrapper> l{7};
    Any d{9};
    std::cout << l.d() << d.d();
}

#endif
