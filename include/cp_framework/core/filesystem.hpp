#pragma once
#include <memory>
#include <span>
#include <cstdint>
#include "export.hpp"
#include "types.hpp"

/**
 * @defgroup Filesystem Filesystem Utilities
 * @brief Cross-platform filesystem and file I/O utilities.
 *
 * This module includes:
 * - Memory-mapped file support (Windows + POSIX)
 * - Normalization and management of game paths
 * - Binary file reading/writing helpers
 * @{
 */

/**
 * @defgroup MMap Memory-Mapped File
 * @ingroup Filesystem
 * @brief RAII wrapper for cross-platform memory-mapped file access.
 * @{
 */

namespace cp::filesystem
{

    /**
     * @class MMapFile
     * @brief RAII wrapper for memory-mapped file access.
     *
     * Provides a cross-platform abstraction for mapping a file into memory.
     * Supports both Windows (WIN32 API) and POSIX `mmap`. The file is automatically
     * unmapped when the object is destroyed.
     *
     * @ingroup MMap
     */
    class MMapFile
    {
    public:
        /** @brief Default constructor (creates an empty, unopened mapping). */
        MMapFile() = default;

        /** @brief Destructor. Automatically releases any mapped file. */
        ~MMapFile() noexcept;

        /** @brief Deleted copy constructor (mappings cannot be duplicated). */
        MMapFile(const MMapFile &) = delete;

        /** @brief Deleted copy assignment operator. */
        MMapFile &operator=(const MMapFile &) = delete;

        /** @brief Move constructor. */
        MMapFile(MMapFile &&other) noexcept;

        /** @brief Move assignment operator. */
        MMapFile &operator=(MMapFile &&other) noexcept;

        /**
         * @brief Opens and memory-maps a file.
         * @param filepath Path to the file to be mapped.
         * @return True on success, false on failure.
         *
         * @ingroup MMap
         */
        bool open(const file_path &filepath) noexcept;

        /**
         * @brief Releases the mapped file, if any.
         * @ingroup MMap
         */
        void release() noexcept;

        /**
         * @brief Gets a raw pointer to the mapped memory.
         * @return Pointer to the mapped data, or nullptr if not mapped.
         *
         * @ingroup MMap
         */
        [[nodiscard]] void *data() const noexcept { return m_data; }

        /**
         * @brief Gets the size of the mapped region.
         * @return Number of bytes mapped.
         *
         * @ingroup MMap
         */
        [[nodiscard]] size_t size() const noexcept { return m_size; }

    private:
#ifdef _WIN32
        void *m_data = nullptr;      ///< Pointer to mapped memory.
        void *m_handle = nullptr;    ///< File handle for Windows.
        void *m_mapHandle = nullptr; ///< Mapping handle for Windows.
#else
        void *m_data = nullptr; ///< Pointer to mapped memory.
        int m_fd = -1;          ///< File descriptor for POSIX systems.
#endif
        size_t m_size = 0; ///< Size of the mapped file.
    };

    /** @} */ // end of MMap group

    // -------------------------------------------------------
    // General filesystem utilities
    // -------------------------------------------------------

    /**
     * @brief Normalizes a filesystem path (removes redundant separators, resolves "." and "..").
     * @param path The input path.
     * @return Normalized path.
     *
     * @ingroup Filesystem
     */
    file_path NormalizePath(const file_path &path) noexcept;

    /**
     * @brief Sets the global game data directory.
     * @param path The path to the game directory.
     *
     * @ingroup Filesystem
     */
    void SetGamePath(const file_path &path);

    /**
     * @brief Retrieves the global game data directory.
     * @return The currently stored game path.
     *
     * @ingroup Filesystem
     */
    file_path GetGamePath();

    // -------------------------------------------------------
    // File operations
    // -------------------------------------------------------

    /**
     * @brief Reads the entire file into memory.
     *
     * Allocates a shared buffer containing the file bytes.
     *
     * @param path Path to the file.
     * @param outSize Output variable receiving the number of bytes read.
     * @return Shared pointer containing the file data, or nullptr on failure.
     *
     * @ingroup Filesystem
     */
    std::shared_ptr<uint8_t[]> ReadBytes(const file_path &path, size_t &outSize);

    /**
     * @brief Reads file bytes and also returns a span view of the data.
     *
     * Useful when you want both ownership (shared_ptr) and a cheap, non-owning view (span).
     *
     * @param path Path to the file.
     * @return Pair of (shared_ptr to buffer, span view over the same data).
     *
     * @ingroup Filesystem
     */
    std::pair<std::shared_ptr<uint8_t[]>, std::span<const uint8_t>> ReadBytesAuto(const file_path &path);

    /**
     * @brief Writes binary data to a file.
     *
     * @param path Target file path.
     * @param data Bytes to write.
     * @param append If true, appends instead of overwriting.
     *
     * @ingroup Filesystem
     */
    void WriteBytes(const file_path &path, std::span<const uint8_t> data, bool append = false);

    /**
     * @brief Checks if a file exists.
     * @param path File path.
     * @return True if the file exists, false otherwise.
     *
     * @ingroup Filesystem
     */
    bool FileExists(const file_path &path) noexcept;

    /**
     * @brief Attempts to delete a file safely.
     * @param path File path.
     * @return True if deletion succeeded, false otherwise.
     *
     * @ingroup Filesystem
     */
    bool DeleteFileSafe(const file_path &path) noexcept;

} // namespace cp::filesystem

/** @} */ // end of Filesystem group
