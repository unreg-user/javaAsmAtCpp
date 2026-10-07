module;

#include <optional>

export module parser_b2;

import array_parsers;
import parser_b1;
import parser_concepts;
import read_utils;

export {
    template <typename Base, typename Cont, typename SpecR, typename SpecW> requires parser::JaacParsable<Base, SpecR, SpecW>
    struct HasAttrInfoMxn : Base {
        template <typename SpecR>
        static std::optional<HasAttrInfoMxn> parse(parser::FileReadPtrBySpec<SpecR>& readPtr) {
            auto access_flags = readPtr.template parse<parser::AccessFlagsDefT>();
            auto name_idx = readPtr.template parse_rev<parser::CPoolSizeT>();
            auto descriptor_idx = readPtr.template parse_rev<parser::CPoolSizeT>();
            auto attrInterval = parser::parse_att_interval(readPtr);
            return access_flags && name_idx && descriptor_idx && attrInterval.i
                           ? std::make_optional(parser::common::FieldInfo{.access_flags = *access_flags,
                                                          .name_idx = *name_idx,
                                                          .descriptor_idx = *descriptor_idx,
                                                          .attrInfo = attrInterval.readSpan()})
                           : std::nullopt;
        }

        template <typename SpecR>
        static bool check_and_move(parser::FileReadPtrBySpec<SpecR>& readPtr) {
            return reader::utils::check_and_move<parser::AccessFlagsDefT, parser::CPoolSizeT, parser::CPoolSizeT>(readPtr) &&
                   parser::parse_att_interval(readPtr).i;
        }

        template <typename SpecW>
        bool deser(parser::FileWritePtrBySpec<SpecW>& readPtr) {
            return ;
        }
    };
}