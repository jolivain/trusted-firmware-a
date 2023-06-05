/*
 * Copyright (c) 2019-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_TBBR_TBBR_IMG_DEF_EXP_H
#define ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_TBBR_TBBR_IMG_DEF_EXP_H

/* EXPORT HEADER -- See include/export/README for details! -- EXPORT HEADER */

#include "../../lib/utils_def_exp.h"

/* Firmware Image Package */
#define FIP_IMAGE_ID			U(0)

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_ID			U(1)

/* SCP Firmware SCP_BL2 */
#define SCP_BL2_IMAGE_ID		U(2)

/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_ID			U(3)

/* Secure Payload BL32 (Trusted OS) */
#define BL32_IMAGE_ID			U(4)

/* Non-Trusted Firmware BL33 */
#define BL33_IMAGE_ID			U(5)

/* Certificates */
#define TRUSTED_BOOT_FW_CERT_ID		U(6)
#define TRUSTED_KEY_CERT_ID		U(7)

#define SCP_FW_KEY_CERT_ID		U(8)
#define SOC_FW_KEY_CERT_ID		U(9)
#define TRUSTED_OS_FW_KEY_CERT_ID	U(10)
#define NON_TRUSTED_FW_KEY_CERT_ID	U(11)

#define SCP_FW_CONTENT_CERT_ID		U(12)
#define SOC_FW_CONTENT_CERT_ID		U(13)
#define TRUSTED_OS_FW_CONTENT_CERT_ID	U(14)
#define NON_TRUSTED_FW_CONTENT_CERT_ID	U(15)

/* Non-Trusted ROM Firmware NS_BL1U */
#define NS_BL1U_IMAGE_ID		U(16)

/* Trusted FWU Certificate */
#define FWU_CERT_ID			U(17)

/* Trusted FWU SCP Firmware SCP_BL2U */
#define SCP_BL2U_IMAGE_ID		U(18)

/* Trusted FWU Boot Firmware BL2U */
#define BL2U_IMAGE_ID			U(19)

/* Non-Trusted FWU Firmware NS_BL2U */
#define NS_BL2U_IMAGE_ID		U(20)

/* Secure Payload BL32_EXTRA1 (Trusted OS Extra1) */
#define BL32_EXTRA1_IMAGE_ID		U(21)

/* Secure Payload BL32_EXTRA2 (Trusted OS Extra2) */
#define BL32_EXTRA2_IMAGE_ID		U(22)

/* HW_CONFIG (e.g. Kernel DT) */
#define HW_CONFIG_ID			U(23)

/* TB_FW_CONFIG */
#define TB_FW_CONFIG_ID			U(24)

/* SOC_FW_CONFIG */
#define SOC_FW_CONFIG_ID		U(25)

/* TOS_FW_CONFIG */
#define TOS_FW_CONFIG_ID		U(26)

/* NT_FW_CONFIG */
#define NT_FW_CONFIG_ID			U(27)

/* GPT primary header and entries */
#define GPT_IMAGE_ID			U(28)

/* GPT backup header and entries */
#define BKUP_GPT_IMAGE_ID		U(29)

/* Binary with STM32 header */
#define STM32_IMAGE_ID			U(30)

/* Encrypted image identifier */
#define ENC_IMAGE_ID			U(31)

/* FW_CONFIG */
#define FW_CONFIG_ID			U(32)

/*
 * Primary FWU metadata image ID
 */
#define FWU_METADATA_IMAGE_ID		U(33)

/*
 * Backup FWU metadata image ID
 */
#define BKUP_FWU_METADATA_IMAGE_ID	U(34)

/* Realm Monitor Manager (RMM) */
#define RMM_IMAGE_ID			U(35)

/* CCA Content Certificate ID */
#define CCA_CONTENT_CERT_ID		U(36)

/* Core SWD Key Certificate ID */
#define CORE_SWD_KEY_CERT_ID		U(37)

/* Platform Key Certificate ID */
#define PLAT_KEY_CERT_ID		U(38)

/* Max Images */
#define MAX_IMAGE_IDS			U(39)

/*
 * Each event log entry has some metadata (i.e. a string) that identifies
 * what is measured.These macros define these strings.
 * Note that these strings follow the standardization recommendations
 * defined in the Arm Server Base Security Guide (a.k.a. SBSG, Arm DEN 0086),
 * where applicable. They should not be changed in the code.
 * Where the SBSG does not make recommendations, we are free to choose any
 * naming convention.
 * The key thing is to choose meaningful strings so that when the TPM event
 * log is used in attestation, the different components can be identified.
 */
#define EVLOG_BL2_STRING		"BL_2"
#define EVLOG_BL31_STRING		"SECURE_RT_EL3"
#if defined(SPD_opteed)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_OPTEE"
#elif defined(SPD_tspd)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_TSPD"
#elif defined(SPD_tlkd)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_TLKD"
#elif defined(SPD_trusty)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_TRUSTY"
#else
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_UNKNOWN"
#endif
#define	EVLOG_BL32_EXTRA1_STRING	"SECURE_RT_EL1_OPTEE_EXTRA1"
#define	EVLOG_BL32_EXTRA2_STRING	"SECURE_RT_EL1_OPTEE_EXTRA2"
#define EVLOG_BL33_STRING		"BL_33"
#define EVLOG_FW_CONFIG_STRING		"FW_CONFIG"
#define EVLOG_HW_CONFIG_STRING		"HW_CONFIG"
#define EVLOG_NT_FW_CONFIG_STRING	"NT_FW_CONFIG"
#define EVLOG_SCP_BL2_STRING		"SYS_CTRL_2"
#define EVLOG_SOC_FW_CONFIG_STRING	"SOC_FW_CONFIG"
#define EVLOG_STM32_STRING		"STM32"
#define EVLOG_TB_FW_CONFIG_STRING	"TB_FW_CONFIG"
#define	EVLOG_TOS_FW_CONFIG_STRING	"TOS_FW_CONFIG"
#define EVLOG_RMM_STRING 		"RMM"
#define EVLOG_SP1_STRING		"SP1"
#define EVLOG_SP2_STRING		"SP2"
#define EVLOG_SP3_STRING		"SP3"
#define EVLOG_SP4_STRING		"SP4"
#define EVLOG_SP5_STRING		"SP5"
#define EVLOG_SP6_STRING		"SP6"
#define EVLOG_SP7_STRING		"SP7"
#define EVLOG_SP8_STRING		"SP8"

#endif /* ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_TBBR_TBBR_IMG_DEF_EXP_H */
