/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RSS_DELEGATED_ATTESTATION_H
#define RSS_DELEGATED_ATTESTATION_H

#include <stdint.h>
#include <psa/crypto_types.h>

#define CCA_PLAT_ATTEST_TOKEN_MAX_SIZE  0x800

psa_status_t
delegated_attestation_get_key(uintptr_t buf, size_t *len,
		unsigned int type);
psa_status_t
delegated_attestation_get_token(uintptr_t buf, size_t *len, uintptr_t hash,
		size_t hash_size);

#endif /* RSS_DELEGATED_ATTESTATION_H */
