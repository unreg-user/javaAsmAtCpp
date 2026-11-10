#pragma once

// clang-format off

#define DEF_DEF_CREATE(mPrefix, Self) \
    template <typename SpecR>\
    NODISCARD static std::optional<Self> mPrefix##parse(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept { \
        return readPtr.template parse_rev<Self>(); \
    }

#define DEF_DEF_CHECK_AND_MOVE(mPrefix, Self) \
    template <typename SpecR> \
    NODISCARD static constexpr bool mPrefix##check_and_move(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept { \
        return readPtr.template check_and_move<Self>(); \
    }

#define DEF_DEF_DESER_MTH(mPrefix, ...) \
    template <typename SpecW> \
    void mPrefix##deser(parser::WritePtrBySpec<SpecW>& writePtr) const noexcept { \
        reader::utils::deser_rev(writePtr, __VA_ARGS__); \
    }

#define DEF_CREATE(Self) DEF_DEF_CREATE(, Self)
#define DEF_CHECK_AND_MOVE(Self) DEF_DEF_CHECK_AND_MOVE(, Self)
#define DEF_DESER_MTH(...) DEF_DEF_DESER_MTH(, __VA_ARGS__)

#define DEF_R_METHODS(Self) DEF_CREATE(Self) DEF_CHECK_AND_MOVE(Self)
#define DEF_METHODS(Self, ...) DEF_CREATE(Self) DEF_CHECK_AND_MOVE(Self) DEF_DESER_MTH(__VA_ARGS__)

#define DEF_TAG_FLAG(tagId) static constexpr TagIdTypeT TAG_ID = opcodes::TagIdEnum::tagId;
#define TAG_FLAG(tagId) static constexpr TagIdTypeT TAG_ID = tagId;

#define DEF_ABS_CREATE(Self) DEF_DEF_CREATE(abs_, Self)
#define DEF_ABS_CHECK_AND_MOVE(Self) DEF_DEF_CHECK_AND_MOVE(abs_, Self)
#define DEF_ABS_DESER(...) DEF_DEF_DESER_MTH(abs_ , __VA_ARGS__)

/** read */
#define DEF_ABS_R_METHODS(Self) DEF_ABS_CREATE(Self) DEF_ABS_CHECK_AND_MOVE(Self)
#define DEF_ABS_METHODS_PACK(Self, ...) protected: DEF_ABS_R_METHODS(Self) public: DEF_DESER_MTH(__VA_ARGS__)

#define DEF_ABS_EXT_CREATE(Self) \
    template <typename SpecR> \
    NODISCARD static std::optional<Self> parse(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept { \
        return Self::template abs_parse<SpecR>(readPtr).transform(cstd::ext_cast_lambda<Self>); \
    }

#define DEF_ABS_EXT_CHECK_AND_MOVE(Self) \
    template <typename SpecR> \
    NODISCARD static constexpr bool check_and_move(parser::ReadPtrBySpec<SpecR>& readPtr) noexcept { \
        return Self::template abs_check_and_move<SpecR>(readPtr); \
    }

#define DEF_ABS_EXT_R_METHODS(Self) DEF_ABS_EXT_CREATE(Self) DEF_ABS_EXT_CHECK_AND_MOVE(Self)

#define SWITCH_TAG_CASE_GEN_2COMPL(instance, trait)  \
    case instance: \
    return std::optional{callable.template operator()<instance, traits::trait>()};

#define SWITCH_TAG_CASE_GEN_1COMPL_1RT(instance, trait)  \
    case instance: \
    return std::optional{callable.template operator()<traits::trait>(instance)};

#define SWITCH_TAG_CASE_GEN_1COMPL(instance)  \
    case instance: \
    return std::optional{callable.template operator()<instance>()};

#define SWITCH_TAG_CASE_GEN_TO_STR_V(instance)  \
    case opcodes::TagIdEnum::CONSTANT_##instance: \
        static constexpr std::string_view name_of_##instance = #instance; \
        return std::optional{name_of_##instance};