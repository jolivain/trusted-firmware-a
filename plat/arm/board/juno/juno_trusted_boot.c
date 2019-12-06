/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <drivers/arm/cryptocell/cc_rotpk.h>
#include <lib/cassert.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <tools_share/tbbr_oid.h>

/* SHA256 algorithm */
#define SHA256_BYTES			32

static const unsigned char rotpk_hash_hdr[] =		\
		"\x30\x31\x30\x0D\x06\x09\x60\x86\x48"	\
		"\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20";
static const unsigned int rotpk_hash_hdr_len = sizeof(rotpk_hash_hdr) - 1;
static unsigned char rotpk_hash_der[sizeof(rotpk_hash_hdr) - 1 + SHA256_BYTES];

#if !ARM_ROTPK_LOCATION_ID
  #error "ARM_ROTPK_LOCATION_ID not defined"
#endif

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID) \
	|| (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_ECDSA_ID)
	static const unsigned char arm_devel_rotpk_hash[] =	ARM_ROTPK_HASH;
#endif

/*
 * Return the ROTPK hash in the following ASN.1 structure in DER format:
 *
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm         OBJECT IDENTIFIER,
 *     parameters        ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm   AlgorithmIdentifier,
 *     digest            OCTET STRING
 * }
 */
int juno_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	uint8_t *dst;

	assert(key_ptr != NULL);
	assert(key_len != NULL);
	assert(flags != NULL);

	/* Copy the DER header */
	memcpy(rotpk_hash_der, rotpk_hash_hdr, rotpk_hash_hdr_len);
	dst = (uint8_t *)&rotpk_hash_der[rotpk_hash_hdr_len];

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID) \
	|| (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_ECDSA_ID)
	memcpy(dst, arm_devel_rotpk_hash, SHA256_BYTES);
#elif (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_REGS_ID)
	uint32_t *src, tmp;
	unsigned int words, i;

	/*
	 * Append the hash from Trusted Root-Key Storage registers. The hash has
	 * not been written linearly into the registers, so we have to do a bit
	 * of byte swapping:
	 *
	 *     0x00    0x04    0x08    0x0C    0x10    0x14    0x18    0x1C
	 * +---------------------------------------------------------------+
	 * | Reg0  | Reg1  | Reg2  | Reg3  | Reg4  | Reg5  | Reg6  | Reg7  |
	 * +---------------------------------------------------------------+
	 *  | ...                    ... |   | ...                   ...  |
	 *  |       +--------------------+   |                    +-------+
	 *  |       |                        |                    |
	 *  +----------------------------+   +----------------------------+
	 *          |                    |                        |       |
	 *  +-------+                    |   +--------------------+       |
	 *  |                            |   |                            |
	 *  v                            v   v                            v
	 * +---------------------------------------------------------------+
	 * |                               |                               |
	 * +---------------------------------------------------------------+
	 *  0                           15  16                           31
	 *
	 * Additionally, we have to access the registers in 32-bit words
	 */
	words = SHA256_BYTES >> 3;

	/* Swap bytes 0-15 (first four registers) */
	src = (uint32_t *)TZ_PUB_KEY_HASH_BASE;
	for (i = 0 ; i < words ; i++) {
		tmp = src[words - 1 - i];
		/* Words are read in little endian */
		*dst++ = (uint8_t)((tmp >> 24) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 16) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 8) & 0xFF);
		*dst++ = (uint8_t)(tmp & 0xFF);
	}

	/* Swap bytes 16-31 (last four registers) */
	src = (uint32_t *)(TZ_PUB_KEY_HASH_BASE + SHA256_BYTES / 2);
	for (i = 0 ; i < words ; i++) {
		tmp = src[words - 1 - i];
		*dst++ = (uint8_t)((tmp >> 24) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 16) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 8) & 0xFF);
		*dst++ = (uint8_t)(tmp & 0xFF);
	}
#endif /* (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID) \
		  || (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_ECDSA_ID) */

	*key_ptr = (void *)rotpk_hash_der;
	*key_len = (unsigned int)sizeof(rotpk_hash_der);
	*flags = ROTPK_IS_HASH;
	return 0;
}

/*
 * Return the ROTPK hash in the following ASN.1 structure in DER format:
 *
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm         OBJECT IDENTIFIER,
 *     parameters        ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm   AlgorithmIdentifier,
 *     digest            OCTET STRING
 * }
 */
int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
#if !ARM_CRYPTOCELL_INTEG
	return juno_get_rotpk_info(cookie, key_ptr, key_len, flags);
#else
	return arm_cc_get_rotpk_info(cookie, key_ptr, key_len, flags);
#endif
}
