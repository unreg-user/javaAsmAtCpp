module;

#include <cassert>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <span>

#include "macro.h"

export module reader;

import simple_cstd;
import gstd;

export namespace reader {
    NODISCARD F_INLINE FILE* open_file(const std::filesystem::path& path, const std::string_view mode) noexcept {
#ifdef _WIN32
        const std::wstring w_mode(mode.begin(), mode.end());
        return _wfopen(path.c_str(), w_mode.c_str());
#else
        return std::fopen(path.c_str(), mode.data());
#endif
    }

    namespace mixins {
        struct FileDataPMxn {
            void unchecked_close(this auto&& self) noexcept {
                free(self.data);
            }
        };
    } // namespace mixins

    template <typename Sym = uint8_t>
    struct FileData : mixins::FileDataPMxn {
        Sym* data = nullptr;

        explicit constexpr FileData(Sym* data) noexcept : data(data) {
        }

        constexpr FileData() noexcept = default;
    };

    template <typename Sym = uint8_t>
    struct FileDataWithSize : mixins::FileDataPMxn {
        Sym* data = nullptr;
        size_t rawSize = 0;

        FileDataWithSize(FileData<Sym> fileData, const size_t size) noexcept : data(fileData.data), rawSize(size) {
        }

        FileDataWithSize(Sym* data, const size_t size) noexcept : data(data), rawSize(size) {
        }

        constexpr FileDataWithSize() noexcept = default;

        /*implicit*/ operator FileData<Sym>() const noexcept {
            return FileData<Sym>{data};
        }

        /*implicit*/ operator std::span<Sym>() const noexcept {
            assert(rawSize % sizeof(Sym) == 0);
            return {data, rawSize / sizeof(Sym)};
        }
    };

    enum class IfBadSize {
        ASSERT, CHECK_AND_NULL, ADD_ALIGNMENT
    };

    struct RWFileData {
        static constexpr int seekSet = SEEK_SET;
        static constexpr int seekCur = SEEK_CUR;
        static constexpr int seekEnd = SEEK_END;

        std::filesystem::path path;
        FILE* file;

        template <typename Sym = uint8_t, bool returnWithSize = true, bool addEmptySymbolsInEnd = true, IfBadSize ifBadSize = IfBadSize::CHECK_AND_NULL>
        NODISCARD inline auto read_file() const noexcept {
            assert(file != nullptr);

            const size_t size = std::filesystem::file_size(path);

            size_t sizeWithEmptySymbol = size;
            if constexpr (addEmptySymbolsInEnd) {
                sizeWithEmptySymbol += sizeof(Sym);
            }
            if constexpr (ifBadSize == IfBadSize::ASSERT) {
                assert(sizeWithEmptySymbol % sizeof(Sym) == 0);
            } else if constexpr (ifBadSize == IfBadSize::CHECK_AND_NULL) {
                if (sizeWithEmptySymbol % sizeof(Sym) != 0) [[unlikely]] {
                    if constexpr (returnWithSize) {
                        return FileDataWithSize<Sym>{nullptr, 0};
                    } else {
                        return FileData<Sym>{nullptr};
                    }
                }
            } else {
                sizeWithEmptySymbol += (sizeof(Sym) - sizeWithEmptySymbol % sizeof(Sym)) % sizeof(Sym);
            }

            const auto data = std::bit_cast<Sym*>(malloc(sizeWithEmptySymbol));
            fread(data, 1, size, file);

            if constexpr (addEmptySymbolsInEnd) {
                for (size_t i = size; i < sizeWithEmptySymbol; ++i) {
                    data[i] = '\0';
                }
            }

            if constexpr (returnWithSize) {
                return FileDataWithSize<Sym>{data, size};
            } else {
                return FileData<Sym>{data};
            }
        }

        template <typename Sym>
        inline auto raw_write_file(FileDataWithSize<Sym>&& data) const noexcept {
            assert(file != nullptr);
            return fwrite(data.data, sizeof(Sym), data.rawSize, file);
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

    template <bool checkExits = true>
    struct PreRWFileData {
        std::filesystem::path path;

        NODISCARD std::optional<RWFileData> open(const char* mode) const noexcept {
            FILE* file = open_file(path, mode);

            if constexpr (checkExits) {
                if (!file) [[unlikely]] {
                    return std::nullopt;
                }
            }

            return std::optional{RWFileData{.path = path, .file = file}};
        }
    };

    struct ReadPtrDefaultFlagsStruct {
        CSFLAG CheckMemorySize = true;
    };

    struct UncheckedReadPtrFlagsStruct {
        CSFLAG CheckMemorySize = false;
    };

    template <typename Sym, typename FlagsStruct = ReadPtrDefaultFlagsStruct>
    struct ReadPtr {
        Sym* i;
        Sym* endPtr;

        /*implicit*/ ReadPtr(std::span<Sym> data) noexcept : i(data.data()), endPtr(data.data() + data.size()) {
        }

        ReadPtr(Sym* i, Sym* end) noexcept : i(i), endPtr(end) {
        }

        ReadPtr(const ReadPtr& other) noexcept : i(other.i), endPtr(other.endPtr) {
        }

        ReadPtr(ReadPtr&& other) noexcept : i(other.i), endPtr(other.endPtr) {
        }

        FAKE_TEMPL requires (!FlagsStruct::CheckMemorySize)
        explicit ReadPtr(Sym* i) noexcept : i(i), endPtr(nullptr) {
        }

        ReadPtr cpyWithSizeSymbols(const size_t size) {
            if constexpr (FlagsStruct::CheckMemorySize)
                assert(i + size < endPtr);
            return {i, i + size};
        }

        ReadPtr cpyWithSize(const size_t bytes) {
            assert(bytes % sizeof(Sym) == 0);
            return cpyWithSizeSymbols(bytes / sizeof(Sym));
        }

        auto readSpan() {
            assert(i <= endPtr);
            auto span = std::span<Sym>{i, static_cast<size_t>(endPtr - i)};
            i = endPtr;
            return span;
        }

        template <typename Type>
        NODISCARD F_INLINE bool memcpy(Type& to) noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot containing in symbols");
            static constexpr auto typeS = sizeof(Type) / sizeof(Sym);

            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!(i + typeS <= endPtr)) [[unlikely]] {
                    return false;
                }
            }

            std::memcpy(&to, i, sizeof(Type));
            i += typeS;
            return true;
        }

        template <typename Type, typename Callable>
        NODISCARD F_INLINE std::optional<Type> parse_via_func(Callable&& callable) noexcept {
            Type to;
            const bool cpyResult = memcpy(to);

            if constexpr (FlagsStruct::CheckMemorySize) {
                if (!cpyResult) [[unlikely]] {
                    return std::nullopt;
                }
            }

            if constexpr (std::same_as<Callable, cstd::identify_lambda_t>) {
                return std::optional{std::move(to)};
            } else {
                return std::optional{callable(std::move(to))};
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
                if (!(i + symbols <= endPtr)) [[unlikely]] {
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
                if (!(i + symbols <= endPtr)) [[unlikely]] {
                    return false;
                }
            }

            i += symbols;
            return true;
        }

        template <size_t bytes>
        NODISCARD F_INLINE bool check_and_move() noexcept {
            static_assert(bytes % sizeof(Sym) == 0, "cannot contain in symbols");
            return check_and_move_symbols<bytes / sizeof(Sym)>();
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
            assert(endPtr);
            return endPtr;
        }

        Sym* cbegin() const noexcept {
            return i;
        }

        Sym* cend() const noexcept {
            assert(endPtr);
            return endPtr;
        }

        /*implicit*/ operator std::span<Sym>() const noexcept {
            ReadPtr copy = *this;
            return copy.readSpan();
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

        template <typename RangeT>
        F_INLINE gstd::true_void deser_range(RangeT&& from) noexcept {
            vec.append_range(std::forward<RangeT>(from));
            return {};
        }

        template <typename Type>
        F_INLINE gstd::true_void deser_via_func(Type&& from, auto&& callable) noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot containing in symbols");

            auto handled = callable(from);
            auto range = std::span<const Sym, sizeof(Type) / sizeof(Sym)>(reinterpret_cast<const Sym*>(&handled),
                                                                          sizeof(Type) / sizeof(Sym));
            vec.append_range(range);
            return {};
        }

        template <typename Type>
        F_INLINE gstd::true_void deser(Type&& from) noexcept {
            deser_via_func(from, cstd::identify_lambda);
            return {};
        }

        template <typename Type>
        F_INLINE gstd::true_void deser_rev(Type&& from) noexcept {
            deser_via_func(from, cstd::byteswap_v_func);
            return {};
        }
    };
} // namespace reader
