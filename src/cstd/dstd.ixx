module;

#include <type_traits>
#include "macro.h"

export module dstd;

export namespace dstd {
    namespace call {
        template <typename T>
        constexpr bool is_ncap = std::is_empty_v<T>;
    }

    namespace args {
        template <typename T>
        using IdentifyApplicator = T;

        template <typename... Types>
        struct TypesTraits {
            template <size_t index>
            using GetTypeAt = Types...[index];

            template <K1 Applicator, KN Wrapper>
            using WApply = Wrapper<Applicator<Types>...>;

            template <K1 Applicator = IdentifyApplicator>
            using Apply = TypesTraits<Applicator<Types>...>;

            template <typename... Types2>
            using Append = TypesTraits<Types..., Types2...>;

            template <typename... Types2>
            using Prepend = TypesTraits<Types2..., Types...>;

            template <KN Wrapper>
            using Wrap = Wrapper<Types...>;

            template <typename... Types2>
            struct AsOp {
                using Conc = TypesTraits<Types..., Types2...>;
            };

            template <typename DirCallable>
            constexpr static auto apply_lam_args(DirCallable&& callable) noexcept {
                using Callable = std::remove_cvref_t<DirCallable>;
                if constexpr (call::is_ncap<Callable>) {
                    return [](Types... args) { return Callable{}(args...); };
                } else {
                    return [callable = std::forward<DirCallable>(callable)] (Types... args) mutable { return callable(args...); };
                }
            }
        };

        template <typename Instance>
        struct TypeWrapper {
            using InstanceT = Instance;
        };
    } // namespace args

#if 0
    template <KN Base>
    struct InheritanceTraits {
        template <typename>
        struct Get;

        template <typename... Signature>
        struct Get<Base<Signature...>> {
            using SigT = args::TypesTraits<Signature...>;
        };
    };
#endif
} // namespace dstd
