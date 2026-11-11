module;

#include <optional>

#include "../../../cstd/macro.h"

export module parser_b1;

import read_utils;
import parser_utils;
import simple_cstd;
import gstd;

export namespace parser::common {
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
        size_t parseSize = 0;

        NODISCARD size_t jaac_get_parse_size() const noexcept {
            return parseSize;
        }

        void jaac_set_parse_size(const size_t size) noexcept {
            parseSize = size;
        }
    };

    struct Translator {
    protected:
        u8_ptr sourceBegin = nullptr;
        u8_ptr sourceEnd = nullptr;
        size_t sourceSize = 0;

    public:
        constexpr Translator() noexcept = default;

        constexpr void jaac_set_source_data_interval_begin(uint8_t* pos) noexcept {
            sourceBegin = pos;
        }

        constexpr void jaac_set_source_data_interval_end(uint8_t* pos) noexcept {
            sourceEnd = pos;
        }

        constexpr void jaac_set_source_data_size(const size_t iSize) noexcept {
            sourceSize = iSize;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_interval_begin(this auto&& self) noexcept {
            return self.sourceBegin;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_interval_end(this auto&& self) noexcept {
            return self.sourceEnd;
        }

        NODISCARD constexpr decltype(auto) jaac_get_source_data_size(this auto&& self) noexcept {
            return self.sourceSize;
        }
    };

    template <typename Base, typename ClassNT>
    CPP_KOSTYL struct ClassNodeKnowsWMxn : Base {
        ClassNT& classNode;

        template <typename... Args> requires (sizeof...(Args) > 0)
        /*implicit*/ constexpr ClassNodeKnowsWMxn(ClassNT& classNode, Args&&... args) noexcept : Base(args...), classNode(classNode) {}

        explicit constexpr ClassNodeKnowsWMxn(ClassNT& classNode) noexcept : Base(), classNode(classNode) {}
    };
} // namespace parser::common
