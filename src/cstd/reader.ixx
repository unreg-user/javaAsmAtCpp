module;

#include <cassert>
#include <cstdio>
#include <expected>
#include <filesystem>

#include "macro.h"

export module reader;

import simple_cstd;

export namespace reader {
    namespace mixins {
        template <typename Sym = uint8_t>
        struct FileDataMxn {
            void unchecked_close(this auto&& self) noexcept {
                free(self.data);
            }
        };
    }

    template <typename Sym = uint8_t>
    struct FileData : mixins::FileDataMxn<Sym> {
        Sym* data;
    };

    template <typename Sym = uint8_t>
    struct FileDataWithSize : mixins::FileDataMxn<Sym> {
        Sym* data;
        size_t size;

        FileDataWithSize(FileData<Sym> fileData, const size_t size) noexcept : data(fileData.data), size(size) {}

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

    template<typename Sym, typename FlagsStruct = ReadPtrDefaultFlagsStruct>
    class ReadPtr {
        Sym* i;
        Sym* last;

    public:
        explicit ReadPtr(FileDataWithSize<Sym> data) noexcept : i(data.data), last(data.data + data.size) {}

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
        NODISCARD F_INLINE std::optional<Type> memcpy_get_via_func(Callable&& callable) noexcept {
            Type to;
            const bool cpyResult = memcpy(&to);

            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!cpyResult) [[unlikely]] {
                    return std::nullopt;
                }
            }

            if constexpr (std::same_as<Callable, cstd::identify_lambda_t>) {
                return {std::move(to)};
            } else {
                return {callable(std::move(to))};
            }

        }

        template <typename Type>
        NODISCARD F_INLINE std::optional<Type> memcpy_get() noexcept {
            return memcpy_get_via_func<Type>(cstd::identify_lambda);
        }

        template <typename Type>
        NODISCARD F_INLINE std::optional<Type> memcpy_get_rev() noexcept {
            return memcpy_get_via_func<Type>(cstd::byteswap_r_func);
        }

        NODISCARD F_INLINE bool check_memory_and_move_symbols(size_t symbols) noexcept {
            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!(i + symbols <= last)) [[unlikely]] {
                    return false;
                }
            }

            i += symbols;
            return true;
        }

        NODISCARD F_INLINE bool check_memory_and_move(const size_t bytes) noexcept {
            assert(bytes % sizeof(Sym) == 0);
            return check_memory_and_move_symbols(bytes / sizeof(Sym));
        }

        template <size_t symbols>
        NODISCARD F_INLINE bool check_memory_and_move_symbols() noexcept {
            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!(i + symbols <= last)) [[unlikely]] {
                    return false;
                }
            }

            i += symbols;
            return true;
        }

        template <size_t bytes>
        NODISCARD F_INLINE bool check_memory_and_move() noexcept {
            static_assert(bytes % sizeof(Sym) == 0, "cannot contain in symbols");
            return check_memory_and_move_symbols<bytes>();
        }

        template <typename Type>
        NODISCARD F_INLINE bool check_memory_and_move() noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot contain in symbols");
            return check_memory_and_move<sizeof(Type) / sizeof(Sym)>();
        }

        NODISCARD F_INLINE Sym* getI() const noexcept {
            return i;
        }
    };

    template <typename Sym>
    FileDataWithSize<Sym> convertToFileDataWS(std::vector<Sym>&& vec) noexcept {
        return FileDataWithSize<Sym>{vec.data(), vec.size()};
    }
}