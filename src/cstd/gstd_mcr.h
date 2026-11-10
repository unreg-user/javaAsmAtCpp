#pragma once

CPP_KOSTYL
#define WIDSTRUCT_USING(Self, T, Base) \
    public: \
        template <typename Y> requires (!std::same_as<Y, const T>) && cstd::is_implicit_convertable<const T, Y> \
        /*implicit*/ constexpr operator Y() const noexcept { \
            return this->Base::instance; \
        } \
        template <typename Y> requires (!cstd::is_implicit_convertable<const T, Y>) && std::constructible_from<Y, const T> \
        explicit constexpr operator Y() const noexcept { \
            return this->Base::instance; \
        } \
        template <typename Y> requires (!std::same_as<Y, T>) && cstd::is_implicit_convertable<T, Y> \
        /*implicit*/ constexpr operator Y() noexcept { \
            return this->Base::instance; \
        } \
        template <typename Y> requires (!cstd::is_implicit_convertable<T, Y>) && std::constructible_from<Y, T> \
        explicit constexpr operator Y() noexcept { \
            return this->Base::instance; \
        } \
        template <typename Y> \
        /*implicit*/ constexpr Self(Y any) requires (!std::same_as<Y, T>) && cstd::is_implicit_convertable<Y, T> : Base(any) {} \
        FAKE_TEMPL requires cstd::is_implicit_default_constructable<T> \
        constexpr Self() : Base(T{}) {} \
        FAKE_TEMPL requires std::is_default_constructible_v<T> && (!cstd::is_implicit_default_constructable<T>) \
        explicit constexpr Self() : Base(T{}) {} \
        template <typename... Args> \
        explicit constexpr Self(Args... args) requires (sizeof...(Args) > 1 || (sizeof...(Args) == 1 && !cstd::is_implicit_convertable<Args...[0], T>)) && std::constructible_from<T, Args...> : Base(T{args...}) {} \
        FAKE_TEMPL requires (!cstd::larger_t_ptr<T>) \
        /*implicit*/ constexpr Self(T t) noexcept : Base(t) {} \
        FAKE_TEMPL requires cstd::larger_t_ptr<T> \
        /*implicit*/ constexpr Self(T& t) noexcept : Base(t) {} \
        FAKE_TEMPL requires cstd::larger_t_ptr<T> \
        /*implicit*/ constexpr Self(T&& t) noexcept : Base(std::move(t)) {}
