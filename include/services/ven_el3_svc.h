/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VEN_EL3_SVC_H
#define VEN_EL3_SVC_H

/* VEN_EL3_SMC_32		U(0x87000000) */
/* VEN_EL3_SMC_64		U(0xC7000000) */

#define VEN_EL3_SVC_UID		U(0x8700ff01)
/*				U(0x8200ff02) is reserved */
#define VEN_EL3_SVC_VERSION	U(0x8700ff03)

#define VEN_EL3_SVC_VERSION_MAJOR	U(0x0)
#define VEN_EL3_SVC_VERSION_MINOR	U(0x1)

/* PMF_SMC_GET_TIMESTAMP_32	U(0x82000010) */
/* PMF_SMC_GET_TIMESTAMP_64	U(0xC2000010) */

/* DEBUGFS_SMC_32		U(0x87000020) */
/* DEBUGFS_SMC_64		U(0xC7000020) */

#endif /* VEN_EL3_SVC_H */
