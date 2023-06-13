/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef RSS_PLATFORM_API_H
#define RSS_PLATFORM_API_H

#include <stdint.h>

#include "psa/error.h"

#define RSS_PLATFORM_API_ID_NV_READ       (1010)
#define RSS_PLATFORM_API_ID_NV_INCREMENT  (1011)

/*
 * Increments the given non-volatile (NV) counter by one
 *
 * counter_id	NV counter ID.
 *
 * PSA_SUCCESS if the value is read correctly. Otherwise,
 *	it returns a PSA_ERROR.
 */
psa_status_t
rss_platform_nv_counter_increment(uint32_t counter_id);

/*
 * Reads the given non-volatile (NV) counter
 *
 * counter_id	NV counter ID.
 * size		Size of the buffer to store NV counter value
 *			in bytes.
 * val		Pointer to store the current NV counter value.
 *
 * PSA_SUCCESS if the value is read correctly. Otherwise,
 *	it returns a PSA_ERROR.
 */
psa_status_t
rss_platform_nv_counter_read(uint32_t counter_id,
		uint32_t size, uint8_t *val);

/*
 * Export a public key or the public part of a key pair in binary format.
 *
 * The output of this function can be passed to psa_import_key() to
 * create an object that is equivalent to the public key.
 *
 * This specification supports a single format for each key type.
 * Implementations may support other formats as long as the standard
 * format is supported. Implementations that support other formats
 * should ensure that the formats are clearly unambiguous so as to
 * minimize the risk that an invalid input is accidentally interpreted
 * according to a different format.
 *
 * For standard key types, the output format is as follows:
 * - For RSA public keys (#PSA_KEY_TYPE_RSA_PUBLIC_KEY), the DER encoding of
 *   the representation defined by RFC 3279 &sect;2.3.1 as `RSAPublicKey`.
 *   ```
 *   RSAPublicKey ::= SEQUENCE {
 *      modulus            INTEGER,    -- n
 *      publicExponent     INTEGER  }  -- e
 *   ```
 * - For elliptic curve keys on a twisted Edwards curve (key types for which
 *   #PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY is true and #PSA_KEY_TYPE_ECC_GET_FAMILY
 *   returns #PSA_ECC_FAMILY_TWISTED_EDWARDS), the public key is as defined
 *   by RFC 8032
 *   (a 32-byte string for Edwards25519, a 57-byte string for Edwards448).
 * - For other elliptic curve public keys (key types for which
 *   #PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY is true), the format is the uncompressed
 *   representation defined by SEC1 &sect;2.3.3 as the content of an ECPoint.
 *   Let `m` be the bit size associated with the curve, i.e. the bit size of
 *   `q` for a curve over `F_q`. The representation consists of:
 *      - The byte 0x04;
 *      - `x_P` as a `ceiling(m/8)`-byte string, big-endian;
 *      - `y_P` as a `ceiling(m/8)`-byte string, big-endian.
 * - For Diffie-Hellman key exchange public keys (key types for which
 *   #PSA_KEY_TYPE_IS_DH_PUBLIC_KEY is true),
 *   the format is the representation of the public key `y = g^x mod p` as a
 *   big-endian byte string. The length of the byte string is the length of the
 *   base prime `p` in bytes.
 *
 * Exporting a public key object or the public part of a key pair is
 * always permitted, regardless of the key's usage flags.
 *
 * Input parameters:
 * key		Identifier of the key to export.
 * data_size	Size of the \p data buffer in bytes.
 *
 * Output parameters:
 * data		Buffer where the key data is to be written.
 * data_length	On success, the number of bytes that make up the key data.
 *
 * Return Values:
 * #PSA_SUCCESS
 * #PSA_ERROR_INVALID_HANDLE
 * #PSA_ERROR_INVALID_ARGUMENT
 * 	The key is neither a public key nor a key pair.
 * #PSA_ERROR_NOT_SUPPORTED
 * #PSA_ERROR_BUFFER_TOO_SMALL
 * 	The size of the \p data buffer is too small. You can determine a
 * 	sufficient buffer size by calling
 * #PSA_EXPORT_KEY_OUTPUT_SIZE(#PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(\c type), \c bits)
 * 	where \c type is the key type
 * 	and \c bits is the key size in bits.
 * #PSA_ERROR_COMMUNICATION_FAILURE
 * #PSA_ERROR_HARDWARE_FAILURE
 * #PSA_ERROR_CORRUPTION_DETECTED
 * #PSA_ERROR_STORAGE_FAILURE
 * #PSA_ERROR_INSUFFICIENT_MEMORY
 * #PSA_ERROR_BAD_STATE
 * 	The library has not been previously initialized by psa_crypto_init().
 * 	It is implementation-dependent whether a failure to initialize
 * 	results in this error code.
 */
psa_status_t
rss_platform_key_read(psa_key_id_t key, uint8_t *data,
		size_t data_size, size_t *data_length);

#endif /* RSS_PLATFORM_API_H */
