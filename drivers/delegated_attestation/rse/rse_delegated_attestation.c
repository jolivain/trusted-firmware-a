/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <psa/crypto_sizes.h>
#include <psa/crypto_types.h>
#include <psa/crypto_values.h>

#include <delegated_attestation.h>
#include <rse_delegated_attestation.h>
#include <services/rmmd_svc.h>

psa_status_t
delegated_attestation_get_key(uintptr_t buf, size_t *len, unsigned int type)
{
	size_t buf_len;
	size_t dak_len;
	psa_status_t ret = PSA_SUCCESS;

	/*
	 * Current RMM implementations only support the public key size for
	 * ECC-P384, i.e. ATTEST_KEY_CURVE_ECC_SECP384R1 attestation key.
	 *
	 * This ECC key has following properties:
	 * ecc_curve:	0x12 (PSA_ECC_FAMILY_SECP_R1)
	 * key_bits:	384
	 * hash_alg:	0x02000009 (PSA_ALG_SHA_256)
	 */
	assert(type == ATTEST_KEY_CURVE_ECC_SECP384R1);

	buf_len = (*len > PSA_KEY_EXPORT_ECC_KEY_PAIR_MAX_SIZE(384)) ?
		  PSA_KEY_EXPORT_ECC_KEY_PAIR_MAX_SIZE(384) : (*len);
	ret = rse_delegated_attest_get_delegated_key(PSA_ECC_FAMILY_SECP_R1,
						     384, (uint8_t *)buf,
						     buf_len, &dak_len,
						     PSA_ALG_SHA_256);
	if (ret != PSA_SUCCESS) {
		return ret;
	}

	if (dak_len != PSA_BITS_TO_BYTES(384)) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	*len = dak_len;

	return ret;
}

psa_status_t
delegated_attestation_get_token(uintptr_t buf, size_t *len,
				uintptr_t hash, size_t hash_size)
{
	size_t token_buf_len;
	size_t token_len = 0;
	psa_status_t ret = PSA_SUCCESS;

	/*
	 * Current support only up to token buffer length value
	 * CCA_PLAT_ATTEST_TOKEN_MAX_SIZE.
	 */
	token_buf_len = ((*len) < CCA_PLAT_ATTEST_TOKEN_MAX_SIZE) ?
			(*len) : CCA_PLAT_ATTEST_TOKEN_MAX_SIZE;

	ret = rse_delegated_attest_get_token((const uint8_t *)hash, hash_size,
					     (uint8_t *)buf, token_buf_len,
					     &token_len);
	if (ret != PSA_SUCCESS) {
		return ret;
	}

	*len = token_len;

	return ret;
}
