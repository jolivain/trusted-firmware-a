/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <psa/crypto_types.h>
#include <psa/crypto_values.h>

#include <delegated_attestation.h>
#include <psa/error.h>
#include <services/rmmd_svc.h>

/*
 * Get the delegated realm attestation key through the PSA delegated
 * attestation layer.
 *
 * FVP cannot support RSS hardware at the moment, but it can still mock
 * the RSS implementation of the PSA interface (see PLAT_RSS_NOT_SUPPORTED).
 */
int plat_rmmd_get_cca_realm_attest_key(uintptr_t buf, size_t *len,
				       unsigned int type)
{
	psa_status_t ret;

	/*
	 * As currently the only supported elliptic curve type is
	 * ATTEST_KEY_CURVE_ECC_SECP384R1, the parameters of the
	 * PSA Delegated Attestation interface are fixed.
	 * ecc_curve:	PSA_ECC_FAMILY_SECP_R1
	 * key_bits:	384
	 */
	assert(type == ATTEST_KEY_CURVE_ECC_SECP384R1);

	ret = rss_delegated_attest_get_delegated_key(PSA_ECC_FAMILY_SECP_R1, 384,
						     (uint8_t *)buf, *len, len,
						     PSA_ALG_SHA_256);

	return ret;
}
