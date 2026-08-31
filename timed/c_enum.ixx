module;

#include "../src/cstd/macro.h"

export module c_enum;

import dstd;

export {
    template <typename Type>
    struct c_enum {
        using Type = Type;

        const Type instance;

        // ReSharper disable once CppNonExplicitConversionOperator
        NODISCARD operator Type() const noexcept {
            return instance;
        }

    protected:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        constexpr c_enum(Type instance) noexcept : instance(instance) {
        }
    };

    // ReSharper disable once CppImplicitDefaultConstructorNotAvailable
    template <typename Type, typename Trait = void>
    struct traited_c_enum : virtual c_enum<Type> {
        using RawEnumType = c_enum<Type>;
        using RawEnumType::instance;
        using RawEnumType::Type;
        using TraitType = Trait;
    };
}

#if 0
template <auto enumInstance>
using TraitedCEnumToNode = dstd::args::Node<enumInstance.instance, typename decltype(enumInstance)::Type>;

export {
    /** accepts roaded
     *  @link traited_c_enum @endlink
     *  instances
     */
    template <auto... enumInstances>
    using TraitedCEnumsTrait = dstd::args::MakeListByRoadedTMapBuilder<TraitedCEnumToNode<enumInstances>...>::Build;
}
#endif
