#pragma once

// clang-format off

#define DEF_CREATE(Self) \
       template <typename Spec> \
    NODISCARD static std::optional<Self> create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept { \
        return dataPtr.template memcpy_get_rev<Self>(); \
    }

#define DEF_CHECK_AND_MOVE(Self) \
       template <typename Spec> \
    NODISCARD static constexpr bool check_and_move(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept { \
        return dataPtr.template check_memory_and_move<Self>(); \
    }

#define DEF_METHODS(Self) DEF_CREATE(Self) DEF_CHECK_AND_MOVE(Self)

#define DEF_TAG_FLAG(tagId) static constexpr opcodes::TagIdEnum TAG_ID = opcodes::TagIdEnum::tagId;

#define DEF_ABS_CREATE(Self) \
    template <typename Spec> \
    NODISCARD static std::optional<Self> abs_create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept { \
        return dataPtr.template memcpy_get_rev<Self>(); \
    }

#define DEF_ABS_CHECK_AND_MOVE(Self) \
    template <typename Spec> \
    NODISCARD static constexpr bool abs_check_and_move(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept { \
        return dataPtr.template check_memory_and_move<Self>(); \
    }

#define DEF_ABS_METHODS(Self) DEF_ABS_CREATE(Self) DEF_ABS_CHECK_AND_MOVE(Self)

#define DEF_ABS_EXT_CREATE(Self) \
    template <typename Spec> \
    NODISCARD static std::optional<Self> create(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept { \
        return abs_create<Spec>(dataPtr).transform(cstd::ext_cast_lambda<Self>); \
    }

#define DEF_ABS_EXT_CHECK_AND_MOVE(Self) \
    template <typename Spec> \
    NODISCARD static constexpr bool check_and_move(parser::FileSymPtrBySpec<Spec>& dataPtr) noexcept { \
        return abs_check_and_move<Spec>(dataPtr); \
    }

#define DEF_ABS_EXT_METHODS(Self) DEF_ABS_EXT_CREATE(Self) DEF_ABS_EXT_CHECK_AND_MOVE(Self)

#define DEF_ABS_EXT_STRUCT(name, parent, tagId) struct name : parent { DEF_TAG_FLAG(tagId) DEF_ABS_EXT_METHODS(name) }

#define SWITCH_TAG_CASE_GEN_2COMPL(instance, trait)  \
    case instance: \
    return std::make_optional(callable.template operator()<instance, traits::trait>());

#define SWITCH_TAG_CASE_GEN_1COMPL_1RT(instance, trait)  \
    case instance: \
    return std::make_optional(callable.template operator()<traits::trait>(instance));

#define SWITCH_TAG_CASE_GEN_1COMPL(instance)  \
    case instance: \
    return std::make_optional(callable.template operator()<instance>());

#define SWITCH_TAG_CASE_GEN_TO_STR_V(instance)  \
    case opcodes::TagIdEnum::CONSTANT_##instance: \
        static constexpr std::string_view name_of_##instance = #instance; \
        return std::make_optional(name_of_##instance);