module;

#include <optional>
#include <bit>
#include "../../../cstd/macro.h"
#include "../../../cstd/gstd_mcr.h"

export module parser_wrappers_b2;

import parser_utils;
import simple_cstd;
import gstd;
import costd;

export namespace parser::wrappers {
    template <typename Self, typename T>
    struct NumberStaticMethods {
        template <typename SpecR>
        F_INLINE static std::optional<Self> parse(ReadPtrBySpec<SpecR>& readPtr) {
            auto optT = readPtr.template parse_rev<T>();
            if (!optT) [[unlikely]] return std::nullopt;
            return std::optional{Self{*optT}};
        }

        template <typename SpecR>
        F_INLINE static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) {
            return readPtr.template check_and_move<T>();
        }
    };

    template <typename T, bool strict = false>
    struct Number : gstd::widstruct<T, strict>, NumberStaticMethods<Number<T, strict>, T> {
        template <typename SpecW>
        F_INLINE gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) {
            writePtr.deser_rev(this->instance);
            return {};
        }

    private:
        using Base = gstd::widstruct<T, strict>;
    public:

        WIDSTRUCT_USING(Number, T, Base);
    };

    struct CpoolSizeT : NumberStaticMethods<CpoolSizeT, CpoolRSizeT>, Number<CpoolRSizeT> {
        using NumberStaticMethods<CpoolSizeT, CpoolRSizeT>::parse;
        using NumberStaticMethods<CpoolSizeT, CpoolRSizeT>::check_and_move;

        NODISCARD CpoolRSizeT jaac_convert_to_cpool_idx() const noexcept {
            return this->instance;
        }

        NODISCARD static Number jaac_convert_from_cpool_idx(const CpoolRSizeT idx) noexcept {
            return idx;
        }

        WIDSTRUCT_USING(CpoolSizeT, CpoolRSizeT, Number<CpoolRSizeT>);
    };

    struct AccessFlags : NumberStaticMethods<AccessFlags, RawAccessFlagsT>, Number<RawAccessFlagsT> {
        using NumberStaticMethods<AccessFlags, RawAccessFlagsT>::parse;
        using NumberStaticMethods<AccessFlags, RawAccessFlagsT>::check_and_move;

        WIDSTRUCT_USING(AccessFlags, RawAccessFlagsT, Number<RawAccessFlagsT>);
    };

    template <typename RawPrimUint, typename RawPrimFN>
    struct AnyFNNum : Number<RawPrimUint>, NumberStaticMethods<AnyFNNum<RawPrimUint, RawPrimFN>, RawPrimUint> {
        using Number<RawPrimUint>::Number;
    private:
        using Base = Number<RawPrimUint>;
    public:
        using NumberStaticMethods<AnyFNNum, RawPrimUint>::parse;
        using NumberStaticMethods<AnyFNNum, RawPrimUint>::check_and_move;

        /*implicit*/ operator RawPrimFN() const {
            return std::bit_cast<RawPrimFN>(this->instance);
        }

        /*implicit*/ AnyFNNum(const RawPrimFN f) : Base(std::bit_cast<RawPrimUint>(f)) {}

        //WIDSTRUCT_USING(AnyFNNum, RawPrimUint, Number<RawPrimUint>);
    };

    using JInt = Number<uint32_t>;
    using JFloat = AnyFNNum<uint32_t, f32_t>;
    using JLong = Number<uint64_t>;
    using JDouble = AnyFNNum<uint64_t, f64_t>;

    template <typename T, bool strict = false>
    struct FlatNumber : gstd::widstruct<T, strict> {
    private:
        using Base = gstd::widstruct<T, strict>;
    public:
        template <typename SpecR>
        F_INLINE static std::optional<FlatNumber> parse(ReadPtrBySpec<SpecR>& readPtr) {
            auto optT = readPtr.template parse<T>();
            if (!optT) [[unlikely]] return std::nullopt;
            return std::optional{FlatNumber{*optT}};
        }

        template <typename SpecR>
        F_INLINE static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) {
            return readPtr.template check_and_move<T>();
        }

        template <typename SpecW>
        F_INLINE gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) {
            writePtr.deser(this->instance);
            return {};
        }

        WIDSTRUCT_USING(FlatNumber, T, Base);
    };

    using MagicNumber = Number<uint32_t>;
    using ArraySizeT = Number<ArrayRSizeT>;
    using AttrsSizeT = Number<AttrsRSizeT>;
} // namespace parser::inline common::wrappers
