module;

#include <optional>
#include <bit>
#include "../../../cstd/macro.h"

export module parser_wrappers_b2;

import parser_utils;
import simple_cstd;
import gstd;

export namespace parser::wrappers {
    template <typename T, bool strict = false>
    struct Number : gstd::widstruct<T, strict> {
        using gstd::widstruct<T>::widstruct;

        template <typename SpecR>
        F_INLINE static std::optional<Number> parse(ReadPtrBySpec<SpecR>& readPtr) {
            return std::make_optional(Number{readPtr.template parse_rev<T>()});
        }

        template <typename SpecR>
        F_INLINE static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) {
            return readPtr.template check_and_move<T>();
        }

        template <typename SpecW>
        F_INLINE gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) {
            writePtr.deser_rev(this->instance);
            return {};
        }
    };

    struct CpoolIdx : Number<CpoolSizeT> {
        using Number::Number;

        NODISCARD CpoolSizeT jaac_convert_to_cpool_idx() const noexcept {
            return *this;
        }

        NODISCARD static Number jaac_convert_from_cpool_idx(const CpoolSizeT idx) noexcept {
            return idx;
        }
    };

    struct AccessFlags : Number<AccessFlagsT> {
        using Number::Number;
    };

    template <typename RawPrimUint, typename RawPrimFN>
    struct AnyFNNum : Number<RawPrimUint> {
        using Number<RawPrimUint>::Number;

    private:
        using Base = Number<RawPrimUint>;

        /*implicit*/ operator RawPrimFN() const {
            return std::bit_cast<RawPrimFN>(this->instance);
        }

        /*implicit*/ AnyFNNum(const RawPrimFN f) : Base(std::bit_cast<RawPrimUint>(f)) {}
    };

    using JInt = Number<uint32_t>;
    using JFloat = AnyFNNum<uint32_t, f32_t>;
    using JLong = Number<uint64_t>;
    using JDouble = AnyFNNum<uint64_t, f64_t>;

    template <typename T, bool strict = false>
    struct FlatNumber : gstd::widstruct<T, strict> {
        using gstd::widstruct<T>::widstruct;

        template <typename SpecR>
        F_INLINE static std::optional<FlatNumber> parse(ReadPtrBySpec<SpecR>& readPtr) {
            return std::make_optional(FlatNumber{readPtr.template parse<T>()});
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
    };
} // namespace parser::inline common::wrappers
