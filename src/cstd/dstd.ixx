module;

#include <concepts>
#include "macro.h"

export module dstd;

namespace dstd {
    // NOLINT(*-concat-nested-namespaces)
    namespace args {
        export {
            /*template <typename Ret, typename... = void>
            using GetFirst = Ret;

            template <typename Type>
            struct TrivialCCFunc {
                using Call = Type;
            };

            struct ErrCCFunc;*/

            template <typename T1, typename T2, typename EmptyType = void>
            struct Get1Of2 {
                static_assert(true, "T1 and T2 not empty");
            };

            template <typename T2, typename EmptyType>
            struct Get1Of2<EmptyType, T2, EmptyType> {
                using Result = T2;
            };

            template <typename T1, typename EmptyType>
            struct Get1Of2<T1, EmptyType, EmptyType> {
                using Result = T1;
            };

            template <typename T>
            using IdentifyApplicator = T;

            template <typename... Types>
            struct TypesTraits {
                template <size_t index>
                using GetTypeAt = Types...[index];

                template <K1 Applicator = IdentifyApplicator, KN Wrapper = TypesTraits>
                using Apply = Wrapper<Applicator<Types>...>;

                template <KN Wrapper>
                using Wrap = Wrapper<Types...>;

                template <typename>
                struct Op {
                    static_assert("not type trait");
                };

                /*template <typename... Types2>
                struct Op<TypesTraits<Types2...>> {
                    template <typename EmptyType = void, typename OrElseCCFunc = ErrCCFunc>
                    using Combine = Op<TypesTraits<typename Get1Of2<Types2>::Result...>>;
                };*/
            };

            template <typename Instance>
            struct TypeWrapper {
                GENERIC_ALIAS(Instance);
            };

            template <typename Value, Value... values>
            struct ValuesTraits {
                GENERIC_ALIAS(Value);
                static constexpr Value valuesVs[] = {values...};

                template <template <Value> typename Applicator, KN Wrapper>
                using Apply = Wrapper<Applicator<values>...>;
            };

            template <size_t from, size_t to>
            struct AbstractTypeIterator {
                V_GENERIC_FIELD(from);
                V_GENERIC_FIELD(to);

                static constexpr size_t hasNext = from < to;
            };

            /*template <size_t from, size_t to, K1 ApplicatorByIter, typename... iterated>
            struct SimpleTypeIterator : AbstractTypeIterator<from, to> {
                using AbstractTypeIterator<to, from>::hasNext;

                using Result = std::conditional_t<
                    hasNext,
                    typename SimpleTypeIterator<from + 1, to, ApplicatorByIter, iterated...,
            ApplicatorByIter<SimpleTypeIterator>>::Result, TypesTraits<iterated...>
                >;

                using Result = TypesTraits<ApplicatorByIter<ApplicatorByIter<from:to>;
            };

            template <typename Return, typename Cache>
            struct ConsequentialTypeIteratorPair {
                GENERIC_ALIAS(Return);
                GENERIC_ALIAS(Cache);
            };

            template <size_t from, size_t to, K1 ApplicatorByIter, typename LastIterCache, typename... iterated>
            struct ConsequentialTypeIterator : AbstractTypeIterator<from, to> {
            private:
                using IterPair = ApplicatorByIter<ConsequentialTypeIterator>;
                using AbstractSelf = AbstractTypeIterator<to, from>;

            public:
                using AbstractSelf::hasNext;
                using AbstractSelf::fromV;
                using AbstractSelf::toV;

                using CacheT = LastIterCache;
                using Result = std::conditional_t<
                    hasNext,
                    typename ConsequentialTypeIterator<from + 1, to, ApplicatorByIter, typename IterPair::CacheT,
            iterated..., typename IterPair::ReturnT>::Result, TypesTraits<iterated...>
                >;
            };*/
        }

        /*namespace {
            template <size_t nodeIndex>
            struct ListByRoadedTMapBuilderCache {
                V_GENERIC_FIELD(nodeIndex);
            };
        }*/

        /*export {
            template <size_t key, typename Value>
            struct Node {
                V_GENERIC_FIELD(key);
                GENERIC_ALIAS(Value);
            };

            template <size_t size, typename... Nodes>
            struct ListByRoadedTMapBuilder : TypesTraits<Nodes...> {
            private:
                template <typename Iterator>
                static consteval auto build1M() {
                    using LastCache = Iterator::CacheT;
                    static constexpr size_t vaIndex = LastCache::fromV;
                    static constexpr size_t nodeIndex = LastCache::nodeIndexV;
                    using NodeVa = Nodes...[nodeIndex];
                    static constexpr size_t nodeVaIndex = NodeVa::keyV;

                    if constexpr (vaIndex < nodeVaIndex) {
                        return ConsequentialTypeIteratorPair<void, LastCache>{};
                    } else if constexpr (vaIndex == nodeVaIndex) {
                        return ConsequentialTypeIteratorPair<typename NodeVa::ValueT,
    ListByRoadedTMapBuilderCache<nodeIndex + 1>>{}; } else { static_assert(false, "Map is unroaded");
                    }
                }

                template <typename Iterator>
                using Build1 = decltype(build1M<Iterator>());

            public:
                using Build = ConsequentialTypeIterator<0, size, Build1, ListByRoadedTMapBuilderCache<0>>;
            };

            template <size_t size, typename... Nodes>
            using MakeListByRoadedTMapBuilder = ListByRoadedTMapBuilder<Nodes...[sizeof...(Nodes)]::keyV, Nodes...>;
        }
    }*/
    } // namespace args
    export namespace call {
        template <typename Func, typename Ret, typename... Args>
        concept CallableCon = requires(Func&& func, Args&&... args) {
            { FORWARD(func)(FORWARD(args)...) } -> std::convertible_to<Ret>;
        };

        template <typename Func, typename Ret, typename... Args>
        concept CallableConSRet = requires(Func&& func, Args&&... args) {
            { FORWARD(func)(FORWARD(args)...) } -> std::same_as<Ret>;
        };

        template <typename Func, typename... Args>
        concept CallableConARet = requires(Func&& func, Args&&... args) { FORWARD(func)(FORWARD(args)...); };

        template <typename T, typename = void>
        struct CallableTraits;

        template <typename Ret, typename ArgsTraits>
        struct UCallableTraits {
            GENERIC_ALIAS(Ret);
            GENERIC_ALIAS(ArgsTraits);
        };

        template <typename Ret, typename... Args>
        struct CallableTraits<Ret(Args...)> : UCallableTraits<Ret, args::TypesTraits<Args...>> {};

        template <typename Class, typename Ret, typename... Args>
        struct CallableTraits<Ret (Class::*)(Args...)> : CallableTraits<Ret(Args...)> {};

        template <typename Class, typename Ret, typename... Args>
        struct CallableTraits<Ret (Class::*)(Args...) const> : CallableTraits<Ret(Args...)> {};

        template <typename Class, typename Ret, typename... Args>
        struct CallableTraits<Ret (Class::*)(Args...) noexcept> : CallableTraits<Ret(Args...)> {};

        template <typename Class, typename Ret, typename... Args>
        struct CallableTraits<Ret (Class::*)(Args...) const noexcept> : CallableTraits<Ret(Args...)> {};

        template <typename Callable>
        struct CallableTraits<Callable, std::void_t<decltype(&Callable::operator())>>
            : CallableTraits<decltype(&Callable::operator())> {};

        template <typename Callable>
        using MakeCallableTraits = CallableTraits<std::decay_t<Callable>>;
    } // namespace call
} // namespace dstd
