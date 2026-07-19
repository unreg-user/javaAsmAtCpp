#pragma once

#define NODISCARD [[nodiscard]]
#define DEPRECATED(text) [[deprecated(text)]]
#define K1 template <typename> typename

#define GETTER(name)                                                                                                   \
    NODISCARD constexpr auto get_##name() const noexcept {                                                             \
        return name;                                                                                                   \
    };
