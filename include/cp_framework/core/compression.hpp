/**
 * @file Compression.hpp
 * @brief Provides zlib-based compression and decompression utilities.
 *
 * @defgroup Compression Data Compression Utilities
 * @brief Utilities for safe and efficient zlib data compression.
 * @{
 */

#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <span>
#include <limits>
#include "export.hpp"

namespace cp::compression
{

    /**
     * @brief Compresses the given data using zlib and embeds the original size.
     *
     * The resulting buffer format is:
     * @code
     * [8 bytes: original_size (uint64_t)] [compressed data...]
     * @endcode
     *
     * @param data Byte span to be compressed.
     * @param level Compression level (1 = fastest, 9 = best compression,
     *              default = 1 / Z_BEST_SPEED).
     *
     * @return std::vector<uint8_t> A buffer containing the 8-byte header followed
     *         by the compressed payload.
     *
     * @note Returns an empty vector in case of compression failure.
     *
     * @ingroup Compression
     */
    [[nodiscard]] CP_API std::vector<uint8_t> CompressData(std::span<const uint8_t> data,
                                                           int level = 1);

    /**
     * @brief Decompresses data produced by CompressData().
     *
     * The function reads the original uncompressed size stored in the 8-byte header
     * and reconstructs the original data.
     *
     * @param compressedData Span containing the 8-byte header + compressed payload.
     * @param maxAllowedSize Maximum allowed size for decompression (default: 4 GiB),
     *                       used to prevent decompression-bomb attacks.
     *
     * @return std::vector<uint8_t> A vector containing the decompressed data.
     *
     * @note Returns an empty vector if:
     *       - the header is invalid
     *       - the input data is corrupted
     *       - decompression fails
     *       - declared size exceeds @p maxAllowedSize
     *
     * @ingroup Compression
     */
    [[nodiscard]] CP_API std::vector<uint8_t> UncompressData(
        std::span<const uint8_t> compressedData,
        uint64_t maxAllowedSize = 4ULL * 1024 * 1024 * 1024);

} // namespace cp::compression

/** @} */ // end of group Compression
