module;

#include <optional>
#include <span>

#include "../../../cstd/macro.h"

export module parser_b1;

import read_utils;
import parser_utils;

export namespace parser::common {
    struct Translator {
    protected:
        uint8_t* source_first = nullptr;
        uint8_t* source_last = nullptr;
        size_t source_size = 0;

    public:
        constexpr Translator() noexcept = default;

        constexpr void jaac_set_source_data_interval_first(uint8_t* pos) noexcept {
            source_first = pos;
        }

        constexpr void jaac_set_source_data_interval_last(uint8_t* pos) noexcept {
            source_last = pos;
        }

        constexpr void jaac_set_source_data_size(const size_t iSize) noexcept {
            source_size = iSize;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_interval_first(this auto&& self) noexcept {
            return self.source_first;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_interval_last(this auto&& self) noexcept {
            return self.source_last;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_size(this auto&& self) noexcept {
            return self.source_size;
        }
    };

    struct FieldInfo {
        AccessFlagsDefT access_flags;
        CPoolSizeT name_idx, descriptor_idx;

        template <typename SpecR>
        static std::optional<FieldInfo> parse(FileReadPtrBySpec<SpecR>& readPtr) {
            auto access_flags = readPtr.template parse<AccessFlagsDefT>();
            auto name_idx = readPtr.template parse_rev<CPoolSizeT>();
            auto descriptor_idx = readPtr.template parse_rev<CPoolSizeT>();
            auto attrInterval = parse_att_interval(readPtr);
            return access_flags && name_idx && descriptor_idx && attrInterval.i
                           ? std::make_optional(FieldInfo{.access_flags = *access_flags,
                                                          .name_idx = *name_idx,
                                                          .descriptor_idx = *descriptor_idx})
                           : std::nullopt;
        }

        template <typename SpecR>
        static bool check_and_move(FileReadPtrBySpec<SpecR>& readPtr) {
            return reader::utils::check_and_move<AccessFlagsDefT, CPoolSizeT, CPoolSizeT>(readPtr) && parse_att_interval(readPtr).i;
        }

        template <typename SpecW>
        bool back(FileWritePtrBySpec<SpecW>& readPtr) {
            return readPtr.deser(access_flags) && reader::utils::deser_rev(readPtr, name_idx, descriptor_idx);
        }
    };
} // namespace parser::common
