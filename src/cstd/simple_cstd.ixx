module;

export module simple_cstd;

export namespace cstd {
    template <typename... Callables>
    struct overload : Callables... {
        using Callables::operator()...;
    };
}