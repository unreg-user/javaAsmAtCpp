module;

#include <cstdint>
#include <utility>

export module code_attr;

import parser_b1;
import parser_b2;
import parser_wrappers_b2;
import parser_utils;
import gstd;

export namespace parser::mcode {
    using CodeDeclSizeT = wrappers::NumToNum<uint32_t, uint16_t>;
    using CodeSizeT = wrappers::Number<uint16_t>;
    using MethodMaxT = wrappers::Number<uint16_t>;

    struct Maxs : common::SerCombinatorPMxn<Maxs, MethodMaxT, MethodMaxT> {
        MethodMaxT maxStack{};
        MethodMaxT maxLocals{};

        constexpr Maxs() noexcept = default;

        constexpr Maxs(const MethodMaxT maxStack, const MethodMaxT maxLocals) noexcept :
            maxStack(maxStack), maxLocals(maxLocals) {
        }

        template <typename SpecW>
        gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) const noexcept {
            maxStack.deser(writePtr);
            maxLocals.deser(writePtr);
            return {};
        }
    };

    struct Exception : common::SerCombinatorPMxn<Maxs, CodeSizeT, CodeSizeT, CodeSizeT, wrappers::CpoolSizeT> {
        CodeSizeT beginIdx{}, endIdx{}, handlerIdx{};
        wrappers::CpoolSizeT typeIdx{};

        constexpr Exception() noexcept = default;

        constexpr Exception(const CodeSizeT beginIdx, const CodeSizeT endIdx, const CodeSizeT handlerIdx,
                            const wrappers::CpoolSizeT typeIdx) noexcept :
            beginIdx(beginIdx), endIdx(endIdx), handlerIdx(handlerIdx), typeIdx(typeIdx) {
        }

        template <typename SpecW>
        gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) const noexcept {
            beginIdx.deser(writePtr);
            endIdx.deser(writePtr);
            handlerIdx.deser(writePtr);
            typeIdx.deser(writePtr);
            return {};
        }
    };

    using RawException = wrappers::FlatNumber<uint64_t>;

    template <typename CodeCont /*elem complex, size_t ~ CodeDeclSizeT*/,
              typename ExceptionsCont /*elem ~ Exception, size_t ~ CodeSizeT*/, typename AttrsCont>
    struct CodeAttribute : Maxs,
                           common::SerCombinatorPMxn<CodeAttribute<CodeCont, ExceptionsCont, AttrsCont>, Maxs, CodeCont,
                                                     ExceptionsCont, AttrsCont> {
        CodeCont code{};
        ExceptionsCont exceptions{};
        AttrsCont attrs{};

        constexpr CodeAttribute() noexcept = default;

        constexpr CodeAttribute(Maxs&& maxs, CodeCont&& code, ExceptionsCont&& exceptions,
                                AttrsCont&& attrs) noexcept :
            Maxs(maxs), code(std::move(code)), exceptions(std::move(exceptions)), attrs(std::move(attrs)) {
        }

        template <typename SpecW>
        gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) const noexcept {
            Maxs::deser(writePtr);
            code.deser(writePtr);
            exceptions.deser(writePtr);
            attrs.deser(writePtr);
            return {};
        }
    };
} // namespace parser::mcode
