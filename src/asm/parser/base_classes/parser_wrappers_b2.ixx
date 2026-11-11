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

    struct NumberNonstaticMethods {
        template <typename SpecW>
        F_INLINE gstd::true_void deser(this auto self, WritePtrBySpec<SpecW>& writePtr) const {
            writePtr.deser_rev(self.instance);
            return {};
        }
    };

    template <typename Self, typename T>
    struct NumberMethods : NumberStaticMethods<Self, T>, NumberNonstaticMethods {
    };

    template <typename T>
    struct Number : gstd::widstruct<T>, NumberMethods<Number<T>, T> {
    private:
        using Base = gstd::widstruct<T>;
    public:
        WIDSTRUCT_USING(Number, T, Base);
    };


    template <typename T1, typename T2>
    struct NumToNum : Number<T2>, NumberMethods<NumToNum<T1, T2>, T1> {
    private:
        using Base = Number<T2>;
        using Base2 = NumberMethods<NumToNum, T1>;
    public:
        using Base2::check_and_move;
        using Base2::parse;
        using Base2::deser;

        WIDSTRUCT_USING(NumToNum, T2, Base);

        /*implicit*/ constexpr NumToNum(T1 t1) noexcept : Base(static_cast<T2>(t1)) {
        }

        /*implicit*/ operator T1() const noexcept {
            return static_cast<T1>(this->instance);
        }
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

        WIDSTRUCT_USING(AnyFNNum, RawPrimUint, Number<RawPrimUint>);
    };

    using JInt = Number<uint32_t>;
    using JFloat = AnyFNNum<uint32_t, f32_t>;
    using JLong = Number<uint64_t>;
    using JDouble = AnyFNNum<uint64_t, f64_t>;

    template <typename T>
    struct FlatNumber : gstd::widstruct<T> {
    private:
        using Base = gstd::widstruct<T>;
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
        F_INLINE gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) const {
            writePtr.deser(this->instance);
            return {};
        }

        WIDSTRUCT_USING(FlatNumber, T, Base);
    };

    using MagicNumber = Number<uint32_t>;
    using ArraySizeT = Number<ArrayRSizeT>;
    using AttrsInnerSizeT = Number<AttrsInnerRSizeT>;
} // namespace parser::inline common::wrappers
