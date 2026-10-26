module;

#include <optional>
#include <span>

#include "../../../cstd/macro.h"

export module parser_b1;

import read_utils;
import parser_utils;
import simple_cstd;

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

    template <typename Base>
    struct JaacIterableMxn : Base {
        using Base::Base;

        auto jaac_begin(this auto&& self) {
            return self.begin();
        }

        auto jaac_end(this auto&& self) {
            return self.end();
        }
    };

    template <typename IBase>
    struct JaacCollectionMxn : JaacIterableMxn<IBase> {
        using JaacIterableMxn<IBase>::JaacIterableMxn;

        void jaac_reserve(this auto&& self, size_t i) {
            self.reserve(i);
        }

        gstd::true_void jaac_push_back(this auto&& self, auto&& element) {
            self.push_back(std::forward<decltype(element)>(element));
            return {};
        }

        auto jaac_size(this auto&& self) {
            return self.size();
        }
    };
} // namespace parser::common
