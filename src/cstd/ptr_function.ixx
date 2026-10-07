module;

#include <functional>
#include <variant>

#include "macro.h"

export module ptr_function;

import dstd;
import simple_cstd;

namespace cstd {
    template <KN Base, typename FuncSign>
    class v_func_g_wrapper;

    template <typename Ret, typename... Args>
    class stack_func_base {
        friend v_func_g_wrapper<stack_func_base, Ret(Args...)>;

        template <typename Func>
        static constexpr bool isNotSelf = !std::same_as<std::remove_cvref_t<Func>, stack_func_base>;

    protected:
        template <typename Func>
        static constexpr bool isHasThatCallOp = std::is_invocable_r_v<Ret, std::decay_t<Func>&, Args...>;

        using f_ptr = Ret(*)(void*, Args...);

        f_ptr vir_f;
        void* callable;

        template <typename Func> requires isNotSelf<Func>
        explicit constexpr stack_func_base(Func* func) noexcept : vir_f([](void* self, Args... args) -> Ret {
            if constexpr (std::same_as<Ret, void>) {
                (*static_cast<std::remove_cvref_t<Func>*>(self))(cstd::forward_cpy<Args>(args)...);
            } else {
                return (*static_cast<std::remove_cvref_t<Func>*>(self))(cstd::forward_cpy<Args>(args)...);
            } // ReSharper disable once CppNotAllPathsReturnValue
            }), callable(static_cast<void*>(func)) {}
        
    public:
        constexpr Ret operator()(Args... args) const {
            return vir_f(callable, args...);
        }
    };

    template <typename Ret, typename... Args>
    class heap_func_base : public stack_func_base<Ret, Args...> {
        friend v_func_g_wrapper<heap_func_base, Ret(Args...)>;

        using Mybase = stack_func_base<Ret, Args...>;

        template <typename Func>
        static constexpr bool isNotSelf = !std::same_as<std::remove_cvref_t<Func>, heap_func_base>;

    protected:
        using d_ptr = void(*)(void*);
        d_ptr vir_d;

        template <typename Func> requires isNotSelf<Func>
        explicit constexpr heap_func_base(Func&& func) noexcept : Mybase(cstd::copy_to_heap_if_noempty(func)), vir_d([](void* self) { delete static_cast<std::remove_cvref_t<Func>*>(self); }) {}

    public:
        ~heap_func_base() noexcept {
            vir_d(this->callable);
        }

        heap_func_base(heap_func_base&& other) noexcept : Mybase(std::exchange(other.callable, nullptr)), vir_d(other.vir_d) {}
        heap_func_base(const heap_func_base&) = delete;
    };

    template <KN KBase, typename FuncSign>
    class v_func_g_wrapper : public dstd::call::RewrapFuncSignature<KBase, FuncSign> {
        using Mybase = dstd::call::RewrapFuncSignature<KBase, FuncSign>;

        template <typename Func>
        static constexpr bool isNotSelf = !std::same_as<std::remove_cvref_t<Func>, v_func_g_wrapper>;

        template <typename Func>
        static constexpr bool isValid = isNotSelf<Func> && Mybase::template isHasThatCallOp<Func>;

    public:
        template <typename Func> requires isValid<Func>
        /*implicit*/ constexpr v_func_g_wrapper(Func& func) noexcept : Mybase(&func) {
        }

        constexpr v_func_g_wrapper(v_func_g_wrapper& other) noexcept : Mybase(other) {
        }

        constexpr v_func_g_wrapper(v_func_g_wrapper&& other) noexcept : Mybase(other) {
        }
    };

    export template <typename FuncSign>
    using stack_func = v_func_g_wrapper<stack_func_base, FuncSign>;

    export template <typename FuncSign>
    using heap_func = v_func_g_wrapper<heap_func_base, FuncSign>;
}