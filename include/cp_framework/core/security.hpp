/**
 * @defgroup Security Security and Encryption Utilities
 * @brief Provides AES encryption helpers and secure key/IV handling.
 * @{
 */

#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include <span>
#include "export.hpp"

namespace cp::security
{

    // ---------------- AES Constants ----------------

    /** @brief AES-128 key size in bytes. */
    constexpr size_t KEY_SIZE = 16;

    /** @brief AES block size (IV size) in bytes. */
    constexpr size_t IV_SIZE = 16;

    /**
     * @brief Holds AES encryption key and initialization vector.
     * @ingroup Security
     */
    struct SecurityData
    {
        std::array<uint8_t, KEY_SIZE> key{}; ///< AES-128 encryption key.
        std::array<uint8_t, IV_SIZE> iv{};   ///< AES initialization vector.
    };

    /**
     * @brief Encrypts a buffer using AES-CBC mode.
     *
     * @param data Plaintext buffer to encrypt.
     * @param securityData Contains the AES key and IV used for encryption.
     * @return A vector containing the encrypted ciphertext bytes.
     *
     * @ingroup Security
     */
    std::vector<uint8_t> EncryptCBC(std::span<const uint8_t> data,
                                    const SecurityData &securityData);

    /**
     * @brief Decrypts an AES-CBC encrypted buffer.
     *
     * @param encrypted Ciphertext bytes to decrypt.
     * @param securityData Contains the AES key and IV used for decryption.
     * @return A vector containing the decrypted plaintext bytes.
     *
     * @ingroup Security
     */
    std::vector<uint8_t> DecryptCBC(std::span<const uint8_t> encrypted,
                                    const SecurityData &securityData);

    /**
     * @brief Generates a random AES key (128-bit) and IV (128-bit).
     *
     * @return A SecurityData structure containing a randomly generated key and IV.
     *
     * @ingroup Security
     */
    SecurityData GenerateRandomKeyAndIV();

} // namespace cp::security

/** @} */ // end of group Security
