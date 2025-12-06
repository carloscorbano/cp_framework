/**
 * @file MD5.hpp
 * @brief Hashing and encoding utilities (MD5, Hex, Base64).
 *
 * @defgroup Crypto Cryptographic Utilities
 * General cryptography-related helper classes and functions.
 * @{
 */

/**
 * @defgroup Crypto_MD5 MD5 Hashing
 * @brief Implementation of the MD5 hashing algorithm.
 * @ingroup Crypto
 */

/**
 * @defgroup Crypto_Utils Encoding Utilities
 * @brief Hex and Base64 encoding/decoding helpers.
 * @ingroup Crypto
 */

#pragma once

#include <string>
#include <cstdint>
#include <span>
#include <vector>
#include "types.hpp"
#include "export.hpp"

namespace cp::algorithm
{

    /**
     * @class MD5
     * @brief Implements the MD5 hashing algorithm.
     *
     * This class provides a full implementation of the MD5 algorithm,
     * including incremental updates, finalization, and hexadecimal output.
     *
     * Typical usage:
     * @code
     * MD5 md5;
     * md5.update("hello");
     * md5.finalize();
     * string hash = md5.hexdigest();
     * @endcode
     *
     * Or using the convenience helpers:
     * @code
     * auto md5 = MD5::Compute("hello");
     * string hash = md5.hexdigest();
     * @endcode
     *
     * @ingroup Crypto_MD5
     */
    class MD5
    {
    public:
        /**
         * @brief Constructs a new MD5 context.
         */
        MD5();

        /**
         * @brief Updates the hash with raw bytes.
         *
         * @param input Pointer to input data.
         * @param length Number of bytes to process.
         */
        void update(const unsigned char *input, size_t length);

        /**
         * @brief Updates the hash with a string.
         *
         * @param input The string to process.
         */
        void update(const string &input);

        /**
         * @brief Finalizes the MD5 computation.
         *
         * After calling this, no further update() calls should be made.
         */
        void finalize();

        /**
         * @brief Returns the hash as a hexadecimal string.
         *
         * @return 32-character hex digest.
         */
        string hexdigest();

        /**
         * @brief Utility method: compute an MD5 hash directly from a byte span.
         *
         * @param data Input bytes.
         * @return A finalized MD5 instance.
         */
        static MD5 Compute(std::span<const uint8_t> data);

        /**
         * @brief Utility method: compute an MD5 hash from text.
         *
         * @param text String view to hash.
         * @return A finalized MD5 instance.
         */
        static MD5 Compute(string_view text);

    private:
        /**
         * @brief Internal MD5 block transform.
         *
         * @param block 64-byte chunk.
         */
        void transform(const unsigned char block[64]);

        /**
         * @brief Encodes 32-bit integers into bytes (little-endian).
         */
        static void encode(const uint32_t *input, unsigned char *output, size_t length);

        /**
         * @brief Decodes bytes into 32-bit integers (little-endian).
         */
        static void decode(const unsigned char *input, uint32_t *output, size_t length);

        bool finalized;           ///< Whether finalize() has been called.
        unsigned char buffer[64]; ///< Input buffer.
        uint32_t count[2];        ///< Bit counters.
        uint32_t state[4];        ///< Current MD5 state.
        unsigned char digest[16]; ///< Final digest.
    };

    /// ================================
    /// HEX UTILITIES
    /// ================================

    /**
     * @namespace Hex
     * @brief Hexadecimal encoding/decoding utilities.
     *
     * @ingroup Crypto_Utils
     */
    namespace Hex
    {

        /**
         * @brief Converts binary data to a hexadecimal string.
         *
         * @param data Input bytes.
         * @param uppercase Whether to use uppercase A-F.
         * @param prefix Whether to prepend "0x".
         */
        string ToHexString(std::span<const uint8_t> data, bool uppercase = false, bool prefix = false);

        /**
         * @brief Overload: vector version of ToHexString().
         */
        string ToHexString(const std::vector<uint8_t> &data, bool uppercase = false, bool prefix = false);

        /**
         * @brief Converts a hex string into bytes.
         *
         * @param hex The input hex string.
         * @return Vector of decoded bytes.
         */
        std::vector<uint8_t> FromHexString(string_view hex);

        /**
         * @brief Converts a hex string with optional "0x" prefix into bytes.
         */
        std::vector<uint8_t> FromHexStringPrefixed(string_view hex);
    }

    /// ================================
    /// BASE64 UTILITIES
    /// ================================

    /**
     * @namespace Base64
     * @brief Base64 encoding/decoding utilities.
     *
     * Supports standard and URL-safe variants.
     *
     * @ingroup Crypto_Utils
     */
    namespace Base64
    {

        /**
         * @brief Encodes bytes into Base64 text.
         */
        string Base64Encode(std::span<const uint8_t> bytes);

        /**
         * @brief Overload: vector version of Base64Encode().
         */
        string Base64Encode(const std::vector<uint8_t> &bytes);

        /**
         * @brief Encodes text directly into Base64.
         */
        string Base64Encode(string_view text);

        /**
         * @brief Encodes bytes using URL-safe Base64 variant.
         */
        string Base64EncodeUrlSafe(std::span<const uint8_t> bytes);

        /**
         * @brief Encodes text using URL-safe Base64 variant.
         */
        string Base64EncodeUrlSafe(string_view text);

        /**
         * @brief Decodes a Base64 string into bytes.
         */
        std::vector<uint8_t> Base64Decode(string_view encoded);

        /**
         * @brief Decodes a URL-safe Base64 string into bytes.
         */
        std::vector<uint8_t> Base64DecodeUrlSafe(string_view encoded);
    }

} // namespace cp

/** @} */ // end of group Crypto
