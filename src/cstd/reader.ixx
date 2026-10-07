module;

#include <cassert>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <span>

#include "macro.h"

export module reader;

import simple_cstd;

export namespace reader {
    namespace mixins {
        struct FileDataMxn {
            void unchecked_close(this auto&& self) noexcept {
                free(self.data);
            }
        };
    } // namespace mixins

    template <typename Sym = uint8_t>
    struct FileData : mixins::FileDataMxn {
        Sym* data;
    };

    template <typename Sym = uint8_t>
    struct FileDataWithSize : mixins::FileDataMxn {
        Sym* data;
        size_t size;

        FileDataWithSize(FileData<Sym> fileData, const size_t size) noexcept : data(fileData.data), size(size) {
        }

        FileDataWithSize(Sym* data, const size_t size) noexcept : data(data), size(size) {
        }

        explicit operator FileData<Sym>() const noexcept {
            return FileData<Sym>{data};
        }
    };

    struct RWFileData {
        static constexpr int seek_set = SEEK_SET;
        static constexpr int seek_cur = SEEK_CUR;
        static constexpr int seek_end = SEEK_END;

        FILE* file;

        template <typename Sym = uint8_t, bool ReturnWithSize = true, bool AddEmptySymbolInEnd = true>
        NODISCARD inline auto read_file() const noexcept {
            assert(file != nullptr);

            fseek(file, 0, seek_end);
            const size_t size = ftell(file);
            fseek(file, 0, seek_set);

            size_t sizeWithEmptySymbol;
            if constexpr (AddEmptySymbolInEnd) {
                sizeWithEmptySymbol = size + 1;
            } else {
                sizeWithEmptySymbol = size;
            }

            const auto data = std::bit_cast<Sym*>(malloc(sizeWithEmptySymbol));
            fread(data, 1, size, file);

            if constexpr (AddEmptySymbolInEnd) {
                data[size] = '\0';
            }

            if constexpr (ReturnWithSize) {
                return FileDataWithSize<Sym>{data, size};
            } else {
                return FileData<Sym>{data};
            }
        }

        template <typename Sym>
        inline auto raw_write_file(FileDataWithSize<Sym>&& data) const noexcept {
            assert(file != nullptr);
            return fwrite(data.data, sizeof(Sym), data.size, file);
        }

        template <typename Sym>
        inline auto raw_write_file(std::vector<Sym>&& data) const noexcept {
            return raw_write_file(FileDataWithSize<Sym>{data.data(), data.size()});
        }

        F_INLINE auto flush() const noexcept { // NOLINT(*-use-nodiscard)
            assert(file != nullptr);
            return fflush(file);
        }

        F_INLINE auto close() const noexcept { // NOLINT(*-use-nodiscard)
            assert(file != nullptr);
            return fclose(file);
        }
    };

    template <bool CheckExits = true>
    struct PreRWFileData {
        const char* path;

        NODISCARD std::optional<RWFileData> open(const char* mode) const noexcept {
            FILE* file = fopen(path, mode);

            if constexpr (CheckExits) {
                if (!file) [[unlikely]] {
                    return std::nullopt;
                }
            }

            return std::make_optional(RWFileData{file});
        }
    };

    struct ReadPtrDefaultFlagsStruct {
        CSFLAG CheckMemorySize = true;
    };

    template <typename Sym, typename FlagsStruct = ReadPtrDefaultFlagsStruct>
    struct ReadPtr {
        Sym* i;
        Sym* last;

        explicit ReadPtr(FileDataWithSize<Sym> data) noexcept : i(data.data), last(data.data + data.size) {
        }

        explicit ReadPtr(std::span<Sym> data) noexcept : i(data.data()), last(data.data() + data.size()) {
        }

        explicit ReadPtr(const ReadPtr& other) noexcept : i(other.i), last(other.last) {
        }

        ReadPtr(Sym* i, Sym* last) noexcept : i(i), last(last) {
        }

        ReadPtr cpyWithSizeSymbols(const size_t size) {
            assert(i + size <= last);
            return {i, i + size};
        }

        ReadPtr cpyWithSize(const size_t bytes) {
            assert(bytes % sizeof(Sym) == 0);
            return cpyWithSizeSymbols(bytes / sizeof(Sym));
        }

        auto readSpan() {
            assert(i <= last);
            auto span = std::span<Sym>{i, last - i};
            i = last;
            return span;
        }

        template <typename Type>
        NODISCARD F_INLINE bool memcpy(Type* to) noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot containing in symbols");
            static constexpr auto typeS = sizeof(Type) / sizeof(Sym);

            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!(i + typeS <= last)) [[unlikely]] {
                    return false;
                }
            }

            std::memcpy(to, i, typeS);
            i += typeS;
            return true;
        }

        template <typename Type, typename Callable>
        NODISCARD F_INLINE std::optional<Type> parse_via_func(Callable&& callable) noexcept {
            Type to;
            const bool cpyResult = memcpy(&to);

            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!cpyResult) [[unlikely]] {
                    return std::nullopt;
                }
            }

            if constexpr (std::same_as<Callable, cstd::identify_lambda_t>) {
                return std::make_optional(std::move(to));
            } else {
                return std::make_optional(callable(std::move(to)));
            }
        }

        template <typename Type>
        NODISCARD F_INLINE std::optional<Type> parse() noexcept {
            return parse_via_func<Type>(cstd::identify_lambda);
        }

        template <typename Type>
        NODISCARD F_INLINE std::optional<Type> parse_rev() noexcept {
            return parse_via_func<Type>(cstd::byteswap_v_func);
        }

        NODISCARD F_INLINE bool check_and_move_symbols(size_t symbols) noexcept {
            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!(i + symbols <= last)) [[unlikely]] {
                    return false;
                }
            }

            i += symbols;
            return true;
        }

        NODISCARD F_INLINE bool check_and_move(const size_t bytes) noexcept {
            assert(bytes % sizeof(Sym) == 0);
            return check_and_move_symbols(bytes / sizeof(Sym));
        }

        template <size_t symbols>
        NODISCARD F_INLINE bool check_and_move_symbols() noexcept {
            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!(i + symbols <= last)) [[unlikely]] {
                    return false;
                }
            }

            i += symbols;
            return true;
        }

        template <size_t bytes>
        NODISCARD F_INLINE bool check_and_move() noexcept {
            static_assert(bytes % sizeof(Sym) == 0, "cannot contain in symbols");
            return check_and_move_symbols<bytes>();
        }

        template <typename Type>
        NODISCARD F_INLINE bool check_and_move() noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot contain in symbols");
            return check_and_move<sizeof(Type) / sizeof(Sym)>();
        }

        Sym* begin() const noexcept {
            return i;
        }

        Sym* end() const noexcept {
            return last + 1;
        }

        Sym* cbegin() const noexcept {
            return i;
        }

        Sym* cend() const noexcept {
            return last + 1;
        }
    };

    using WriteVecPtrDefaultFlagsStruct = ReadPtrDefaultFlagsStruct;

    template <typename Sym, typename FlagsStruct = WriteVecPtrDefaultFlagsStruct>
    struct WriteVecPtr {
    private:
        std::vector<Sym>& vec;

    public:
        explicit WriteVecPtr(std::vector<Sym>& vec) noexcept : vec(vec) {
        }

        explicit WriteVecPtr(const WriteVecPtr& other) noexcept : vec(other.vec) {
        }

        template <std::_Container_compatible_range<Sym> RangeT>
        NODISCARD F_INLINE void deser_range(RangeT&& from) noexcept {
            vec.append_range(from);
        }

        template <typename Type>
        NODISCARD F_INLINE void deser_via_func(Type&& from, auto&& callable) noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot containing in symbols");

            auto handled = callable(from);
            auto range = std::span<const Sym, sizeof(Type) / sizeof(Sym)>(reinterpret_cast<const Sym*>(&handled),
                                                                          sizeof(Type) / sizeof(Sym));
            vec.append_range(range);
        }

        template <typename Type>
        NODISCARD F_INLINE void deser(Type&& from) noexcept {
            return deser_via_func(from, cstd::identify_lambda);
        }

        template <typename Type>
        NODISCARD F_INLINE void deser_rev(Type&& from) noexcept {
            return deser_via_func(from, cstd::byteswap_v_func);
        }
    };

    template <typename Sym>
    FileDataWithSize<Sym> convertToFileDataWS(std::vector<Sym>&& vec) noexcept {
        return FileDataWithSize<Sym>{vec.data(), vec.size()};
    }
} // namespace reader
