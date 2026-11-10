module;

#include <optional>
#include <span>

#include "../../../cstd/macro.h"

export module parser_b1;

import read_utils;
import parser_utils;
import simple_cstd;
import gstd;

export namespace parser::common {
    struct Translator {
    protected:
        u8_ptr source_first = nullptr;
        u8_ptr source_last = nullptr;
        size_t source_size = 0;

    public:
        constexpr Translator() noexcept = default;

        constexpr void jaac_set_source_data_interval_first(u8_ptr pos) noexcept {
            source_first = pos;
        }

        constexpr void jaac_set_source_data_interval_last(u8_ptr pos) noexcept {
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

    struct JaacIterablePMxn {
        decltype(auto) jaac_begin(this auto&& self) {
            return self.begin();
        }

        decltype(auto) jaac_end(this auto&& self) {
            return self.end();
        }
    };

    struct JaacSpanPMxn : JaacIterablePMxn {
        decltype(auto) jaac_size(this auto&& self) {
            return self.size();
        }
    };

    struct JaacVectorPMxn : JaacSpanPMxn {
        void jaac_reserve(this auto&& self, size_t i) {
            self.reserve(i);
        }

        template <typename Element>
        gstd::true_void jaac_push_back(this auto&& self, Element&& element) {
            self.push_back(std::forward<Element>(element));
            return {};
        }

        template <typename Element>
        gstd::true_void jaac_emplace_back(this auto&& self, Element&& element) {
            self.emplace_back(std::forward<Element>(element));
            return {};
        }
    };

    struct HasParseSizePMxn {
        size_t parse_size = 0;

        NODISCARD size_t jaac_get_parse_size() const noexcept {
            return parse_size;
        }

        void jaac_set_parse_size(const size_t size) noexcept {
            parse_size = size;
        }
    };
} // namespace parser::common
