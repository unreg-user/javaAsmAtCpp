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
    NODISCARD F_INLINE FILE* open_file(const std::filesystem::path& path, const char* rawMode) noexcept {
#ifdef _WIN32
        std::string_view mode = rawMode;
        const std::wstring w_mode(mode.begin(), mode.end());
        return _wfopen(path.c_str(), w_mode.c_str());
#else
        return std::fopen(path.c_str(), rawMode);
#endif
    }

    namespace mixins {
        struct FileDataPMxn {
            void unchecked_close(this auto&& self) noexcept {
                free(std::forward_like<decltype(self)>(self).data);
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

        IMPLICIT operator FileData<Sym>() const noexcept {
            return FileData<Sym>{data};
        }

        IMPLICIT operator std::span<Sym>() const noexcept {
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
        COM_BOOL checkMemorySize = true;
    };

    struct UncheckedReadPtrFlagsStruct {
        COM_BOOL checkMemorySize = false;
    };

    template <typename Sym, typename FlagsStruct = ReadPtrDefaultFlagsStruct>
    struct ReadPtr {
        Sym* i;
        Sym* endPtr;

        IMPLICIT ReadPtr(std::span<Sym> data) noexcept : i(data.data()), endPtr(data.data() + data.size()) {
        }

        ReadPtr(Sym* i, Sym* end) noexcept : i(i), endPtr(end) {
        }

        ReadPtr(const ReadPtr& other) noexcept : i(other.i), endPtr(other.endPtr) {
        }

        ReadPtr(ReadPtr&& other) noexcept : i(other.i), endPtr(other.endPtr) {
        }

        FAKE_TEMPL requires (!FlagsStruct::checkMemorySize)
        explicit ReadPtr(Sym* i) noexcept : i(i), endPtr(nullptr) {
        }

        FAKE_TEMPL requires (!FlagsStruct::checkMemorySize)
        explicit ReadPtr() noexcept : i(nullptr), endPtr(nullptr) {
        }

        ReadPtr cpyWithSizeSymbols(const size_t size) const {
            if constexpr (FlagsStruct::checkMemorySize)
                assert(i + size <= endPtr);
            return {i, i + size};
        }

        ReadPtr cpyWithSize(const size_t bytes) const {
            assert(bytes % sizeof(Sym) == 0);
            return cpyWithSizeSymbols(bytes / sizeof(Sym));
        }

        auto readSpan(this auto&& self) {
            assert(self.i <= self.endPtr);
            auto span = std::span<Sym>{self.i, static_cast<size_t>(self.endPtr - self.i)};
            if constexpr (!std::is_rvalue_reference_v<decltype(self)>) {
                self.i = self.endPtr;
            }
            return span;
        }

        template <typename Type>
        NODISCARD F_INLINE bool memcpy(this auto&& self, Type& to) noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot containing in symbols");
            static constexpr auto typeS = sizeof(Type) / sizeof(Sym);

            if constexpr (FlagsStruct::checkMemorySize) {
                if (!(self.i + typeS <= self.endPtr)) [[unlikely]] {
                    return false;
                }
            }

            std::memcpy(&to, self.i, sizeof(Type));
            if constexpr (!std::is_rvalue_reference_v<decltype(self)>) {
                self.i += typeS;
            }
            return true;
        }

        template <typename Type, typename DirCallable>
        NODISCARD F_INLINE std::optional<Type> parse_via_func(this auto&& self, DirCallable&& callable) noexcept {
            using Callable = std::remove_cvref_t<DirCallable>;

            Type to;
            const bool cpyResult = std::forward_like<decltype(self)>(self).memcpy(to);

            if constexpr (FlagsStruct::checkMemorySize) {
                if (!cpyResult) [[unlikely]] {
                    return std::nullopt;
                }
            }

            if constexpr (std::same_as<Callable, decltype(cstd::identifyLam)>) {
                return std::optional{std::move(to)};
            } else {
                return std::optional{std::forward<DirCallable>(callable)(std::move(to))};
            }
        }

        template <typename Type>
        NODISCARD F_INLINE std::optional<Type> parse(this auto&& self) noexcept {
            return std::forward_like<decltype(self)>(self).template parse_via_func<Type>(cstd::identifyLam);
        }

        template <typename Type>
        NODISCARD F_INLINE std::optional<Type> parse_rev(this auto&& self) noexcept {
            return std::forward_like<decltype(self)>(self).template parse_via_func<Type>(cstd::byteswapLam);
        }

        NODISCARD F_INLINE bool check_and_move_symbols(this auto&& self, size_t symbols) noexcept {
            if constexpr (FlagsStruct::checkMemorySize) {
                if (!(self.i + symbols <= self.endPtr)) [[unlikely]] {
                    return false;
                }
            }

            if constexpr (!std::is_rvalue_reference_v<decltype(self)>) {
                self.i += symbols;
            }

            return true;
        }

        NODISCARD F_INLINE bool check_and_move(this auto&& self, const size_t bytes) noexcept {
            assert(bytes % sizeof(Sym) == 0);
            return std::forward_like<decltype(self)>(self).check_and_move_symbols(bytes / sizeof(Sym));
        }

        template <size_t symbols>
        NODISCARD F_INLINE bool check_and_move_symbols(this auto&& self) noexcept {
            if constexpr (FlagsStruct::checkMemorySize) {
                if (!(self.i + symbols <= self.endPtr)) [[unlikely]] {
                    return false;
                }
            }

            if constexpr (!std::is_rvalue_reference_v<decltype(self)>) {
                self.i += symbols;
            }

            return true;
        }

        template <size_t bytes>
        NODISCARD F_INLINE bool check_and_move(this auto&& self) noexcept {
            static_assert(bytes % sizeof(Sym) == 0, "cannot contain in symbols");
            return std::forward_like<decltype(self)>(self).template check_and_move_symbols<bytes / sizeof(Sym)>();
        }

        template <typename Type>
        NODISCARD F_INLINE bool check_and_move(this auto&& self) noexcept {
            static_assert(sizeof(Type) % sizeof(Sym) == 0, "cannot contain in symbols");
            return std::forward_like<decltype(self)>(self).template check_and_move<sizeof(Type)>();
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

        IMPLICIT operator std::span<Sym>() const noexcept {
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
            deser_via_func(from, cstd::identifyLam);
            return {};
        }

        template <typename Type>
        F_INLINE gstd::true_void deser_rev(Type&& from) noexcept {
            deser_via_func(from, cstd::byteswapLam);
            return {};
        }
    };
} // namespace reader
