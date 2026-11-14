#pragma once

// clang-format off

#define DEF_TAG_FLAG(tagId) static constexpr TagIdTypeT TAG_ID = opcodes::TagIdEnum::tagId;
#define TAG_FLAG(tagId) static constexpr TagIdTypeT TAG_ID = tagId;

#define SWITCH_TAG_CASE_GEN_2COMPL(instance, trait)  \
    case instance: \
    return std::optional{callable.template operator()<instance, traits::trait>()};

#define SWITCH_TAG_CASE_GEN(instance, trait)  \
    case instance: \
    return std::optional{callable.template operator()<traits::trait>()};

#define SWITCH_TAG_CASE_GEN_TO_STR_V(instance)  \
    case opcodes::TagIdEnum::CONSTANT_##instance: \
        static constexpr std::string_view name_of_##instance = #instance; \
        return std::optional{name_of_##instance};