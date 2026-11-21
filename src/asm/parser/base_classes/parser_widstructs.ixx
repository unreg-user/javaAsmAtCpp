module;

#include <optional>
#include <bit>
#include "../../../cstd/macro.h"
#include "../../../cstd/gstd_mcr.h"

export module parser_widstructs;

import parser_utils;
import simple_cstd;
import gstd;
import costd;
import parser_mxns;

export namespace parser::wrappers {
    template <typename T>
    struct Number : gstd::any_widstruct<T> {
        using gstd::any_widstruct<T>::any_widstruct;

    private:
        using Base = gstd::any_widstruct<T>;

    public:

        template <typename SpecW>
        F_INLINE gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) const noexcept {
            writePtr.deser_rev(this->instance);
            return {};
        }

        template <typename SpecR>
        F_INLINE static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return readPtr.template check_and_move<T>();
        }

        template <typename SpecR>
        F_INLINE static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return std::move(readPtr).template check_and_move<T>();
        }

        template <typename SpecR>
        F_INLINE static std::optional<Number> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            auto optT = readPtr.template parse_rev<T>();
            if (!optT) [[unlikely]] return std::nullopt;
            return std::optional{Number{*optT}};
        }

        template <typename SpecR>
        F_INLINE static std::optional<Number> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            auto optT = std::move(readPtr).template parse_rev<T>();
            if (!optT) [[unlikely]] return std::nullopt;
            return std::optional{Number{*optT}};
        }
    };

    template <typename T1, typename T2>
    struct NumToNum : mxns::JaacWidstruct<Number<T2>, NumToNum<T1, T2>> {
        using mxns::JaacWidstruct<Number<T2>, NumToNum>::JaacWidstruct;

    private:
        using Base = mxns::JaacWidstruct<Number<T2>, NumToNum>;
        friend mxns::RewrapperPMxn<Number<T2>, NumToNum>;

    public:
        IMPLICIT constexpr NumToNum(T1 t1) noexcept : Base(static_cast<T2>(t1)) {
        }

        IMPLICIT constexpr operator T1() const noexcept {
            return static_cast<T1>(this->instance);
        }
    };

    struct CpoolSizeT : mxns::JaacWidstruct<Number<CpoolRSizeT>, CpoolSizeT> {
        using JaacWidstruct::JaacWidstruct;
        friend RewrapperPMxn;

        NODISCARD constexpr CpoolRSizeT jaac_convert_to_cpool_idx() const noexcept {
            return this->instance;
        }

        NODISCARD constexpr static Number jaac_convert_from_cpool_idx(const CpoolRSizeT idx) noexcept {
            return idx;
        }
    };

    struct AccessFlags : mxns::JaacWidstruct<Number<RawAccessFlagsT>, AccessFlags> {
        using JaacWidstruct::JaacWidstruct;
        friend RewrapperPMxn;
    };

    template <typename RawPrimUint, typename RawPrimFlN>
    struct AnyFNNum : mxns::JaacWidstruct<Number<RawPrimUint>, AnyFNNum<RawPrimUint, RawPrimFlN>> {
        using mxns::JaacWidstruct<Number<RawPrimUint>, AnyFNNum>::JaacWidstruct;
    private:
        using Base = mxns::JaacWidstruct<Number<RawPrimUint>, AnyFNNum>;
        friend mxns::RewrapperPMxn<Number<RawPrimUint>, AnyFNNum>;
    public:

        IMPLICIT constexpr operator RawPrimFlN() const {
            return std::bit_cast<RawPrimFlN>(this->instance);
        }

        IMPLICIT constexpr AnyFNNum(const RawPrimFlN fl) : Base(std::bit_cast<RawPrimUint>(fl)) {}
    };

    using JInt = Number<uint32_t>;
    using JFloat = AnyFNNum<uint32_t, f32_t>;
    using JLong = Number<uint64_t>;
    using JDouble = AnyFNNum<uint64_t, f64_t>;

    template <typename T>
    struct FlatNumber : gstd::any_widstruct<T> {
        using gstd::any_widstruct<T>::any_widstruct;

    private:
        using Base = gstd::any_widstruct<T>;

    public:
        template <typename SpecR>
        F_INLINE static std::optional<FlatNumber> parse(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            auto optT = readPtr.template parse<T>();
            if (!optT) [[unlikely]] return std::nullopt;
            return std::optional{FlatNumber{*optT}};
        }

        template <typename SpecR>
        F_INLINE static std::optional<FlatNumber> parse(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            auto optT = std::move(readPtr).template parse<T>();
            if (!optT) [[unlikely]] return std::nullopt;
            return std::optional{FlatNumber{*optT}};
        }

        template <typename SpecR>
        F_INLINE static bool check_and_move(ReadPtrBySpec<SpecR>& readPtr) noexcept {
            return readPtr.template check_and_move<T>();
        }

        template <typename SpecR>
        F_INLINE static bool check_and_move(ReadPtrBySpec<SpecR>&& readPtr) noexcept {
            return std::move(readPtr).template check_and_move<T>();
        }

        template <typename SpecW>
        F_INLINE gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) const noexcept {
            writePtr.deser(this->instance);
            return {};
        }
    };

    using MagicNumber = Number<uint32_t>;
    using ArraySizeT = Number<ArrayRSizeT>;
    using AttrsInnerSizeT = Number<AttrsInnerRSizeT>;

    using SVersionNumber = Number<uint16_t>;
    struct VersionNumber : mxns::SerCombinatorPMxn<VersionNumber, SVersionNumber, SVersionNumber> {
        SVersionNumber minor{}, major{};

        constexpr VersionNumber(const SVersionNumber minor, const SVersionNumber major) noexcept :
            minor(minor), major(major) {
        }

        constexpr VersionNumber() noexcept = default;

        template <typename SpecW>
        gstd::true_void deser(WritePtrBySpec<SpecW>& writePtr) const noexcept {
            minor.deser(writePtr);
            major.deser(writePtr);
            return {};
        }
    };
} // namespace parser::wrappers