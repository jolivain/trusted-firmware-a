/*
 * Copyright (c) 2020-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DUALROOT_OID_H
#define DUALROOT_OID_H

/* Reuse the Object IDs defined by TBBR for certificate extensions. */
#include "tbbr_oid.h"
#include "zero_oid.h"

/*
 * Platform root-of-trust public key.
 * Arbitrary value that does not conflict with any of the TBBR reserved OIDs.
 */
#define PROT_PK_OID				"1.3.6.1.4.1.4128.2100.1102"

/*
 * Public Keys present in SOC FW content certificates authenticate BL31 and
 * its configuration. Guarded under dualroot CoT as this file may gets
 * included in other CoTs.
 */
#if defined(ARM_COT_dualroot)
#define BL31_IMAGE_KEY_OID                      SOC_FW_CONTENT_CERT_PK_OID
#define SOC_FW_CONFIG_KEY_OID                   SOC_FW_CONTENT_CERT_PK_OID
#endif /* ARM_COT_dualroot */

#endif /* DUALROOT_OID_H */
