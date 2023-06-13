/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef RSS_CRYPTO_DEFS_H
#define RSS_CRYPTO_DEFS_H

#include <stdint.h>
#include <limits.h>

/* Declares types that encode errors, algorithms, key types, policies, etc. */
#include "psa/crypto_types.h"

/*
 * The persistent key identifiers for RSS builtin keys.
 *
 * The value of RSS_BUILTIN_KEY_ID_MIN (and therefore of the whole range) is
 * completely arbitrary except for being inside the PSA builtin keys range.
 *
 */
enum rss_key_id_builtin_t {
	RSS_BUILTIN_KEY_ID_MIN = 0x7fff815Bu,
	RSS_BUILTIN_KEY_ID_HUK,
	RSS_BUILTIN_KEY_ID_IAK,
#ifdef RSS_PARTITION_DELEGATED_ATTESTATION
	RSS_BUILTIN_KEY_ID_DAK_SEED,
#endif /* RSS_PARTITION_DELEGATED_ATTESTATION */
	RSS_BUILTIN_KEY_ID_PLAT_SPECIFIC_MIN = 0x7FFF816Bu,
	RSS_BUILTIN_KEY_ID_HOST_S_ROTPK,
	RSS_BUILTIN_KEY_ID_HOST_NS_ROTPK,
	RSS_BUILTIN_KEY_ID_HOST_CCA_ROTPK,
	RSS_BUILTIN_KEY_ID_MAX = 0x7FFF817Bu,
};

/*
 * This type is used to overcome a limitation within RSS firmware in the number of maximum
 * IOVECs it can use especially in psa_aead_encrypt and psa_aead_decrypt.
 */
#define RSS_CRYPTO_MAX_NONCE_LENGTH (16u)
struct rss_crypto_aead_pack_input {
	uint8_t nonce[RSS_CRYPTO_MAX_NONCE_LENGTH];
	uint32_t nonce_length;
};

/*
 * Structure used to pack non-pointer types in a call
 */
struct rss_crypto_pack_iovec {
	psa_key_id_t key_id;	/* Key id */
	psa_algorithm_t alg;	/* Algorithm */
	uint32_t op_handle;	/* Frontend context handle associated
				   to a multipart operation */
	uint32_t capacity;	/* Key derivation capacity */
	uint32_t ad_length;	/* Additional Data length for multipart AEAD */
	uint32_t plaintext_length;	/* Plaintext length for multipart AEAD */
	struct rss_crypto_aead_pack_input aead_in;	/* Packs AEAD-related inputs */
	uint16_t function_id;	/* Used to identify the function in the API dispatcher
				   to the service backend. See rss_crypto_func_sid for
				   detail */
	uint16_t step;		/* Key derivation step */
};

/*
 * Type associated to the group of a function encoding. There can be
 * nine groups (Random, Key management, Hash, MAC, Cipher, AEAD, Asym
 * sign, Asym encrypt, Key derivation).
 */
enum rss_crypto_group_id {
	RSS_CRYPTO_GROUP_ID_RANDOM = 0x0,
	RSS_CRYPTO_GROUP_ID_KEY_MANAGEMENT,
	RSS_CRYPTO_GROUP_ID_HASH,
	RSS_CRYPTO_GROUP_ID_MAC,
	RSS_CRYPTO_GROUP_ID_CIPHER,
	RSS_CRYPTO_GROUP_ID_AEAD,
	RSS_CRYPTO_GROUP_ID_ASYM_SIGN,
	RSS_CRYPTO_GROUP_ID_ASYM_ENCRYPT,
	RSS_CRYPTO_GROUP_ID_KEY_DERIVATION,
};

/* X macro describing each of the available PSA Crypto APIs */
#define KEY_MANAGEMENT_FUNCS				\
	X(RSS_CRYPTO_GET_KEY_ATTRIBUTES)		\
	X(RSS_CRYPTO_RESET_KEY_ATTRIBUTES)		\
	X(RSS_CRYPTO_OPEN_KEY)				\
	X(RSS_CRYPTO_CLOSE_KEY)				\
	X(RSS_CRYPTO_IMPORT_KEY)			\
	X(RSS_CRYPTO_DESTROY_KEY)			\
	X(RSS_CRYPTO_EXPORT_KEY)			\
	X(RSS_CRYPTO_EXPORT_PUBLIC_KEY)			\
	X(RSS_CRYPTO_PURGE_KEY)				\
	X(RSS_CRYPTO_COPY_KEY)				\
	X(RSS_CRYPTO_GENERATE_KEY)

/*
 * Define function ID for key management.
 * The function ID will be encoded into rss_crypto_func_sid below.
 */
#define X(func_id)	func_id,
enum rss_crypto_key_management_func_id {
	KEY_MANAGEMENT_FUNCS
};
#undef X
#define FUNC_ID(func_id)	(((func_id) & 0xFF) << 8)

/*
 * Numerical progressive value identifying a function API exposed through
 * the interfaces (S or NS). It's used to dispatch the requests from S/NS
 * to the corresponding API implementation in the Crypto service backend.
 *
 * Each function SID is encoded as uint16_t.
 *     |  Func ID  |  Group ID |
 *     15         8 7          0
 * Func ID is defined in key management func_id enum above
 * Group ID is defined in rss_crypto_group_id.
 */
enum rss_crypto_func_sid {
#define X(func_id)	func_id ## _SID = (uint16_t)((FUNC_ID(func_id)) | \
				(RSS_CRYPTO_GROUP_ID_KEY_MANAGEMENT & 0xFF)),
	KEY_MANAGEMENT_FUNCS
};
#undef X

#endif /* RSS_CRYPTO_DEFS_H */
