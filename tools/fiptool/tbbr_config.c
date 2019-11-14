/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <firmware_image_package.h>

#include "tbbr_config.h"

/* The images used depends on the platform. */
toc_entry_t toc_entries[] = {
	{
		.name = "SCP Firmware Updater Configuration FWU SCP_BL2U",
		.uuid = UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U,
		.cmdline_name = "scp-fwu-cfg",
		.cmdline_enc_name = "scp-fwu-cfg-enc"
	},
	{
		.name = "AP Firmware Updater Configuration BL2U",
		.uuid = UUID_TRUSTED_UPDATE_FIRMWARE_BL2U,
		.cmdline_name = "ap-fwu-cfg",
		.cmdline_enc_name = "ap-fwu-cfg-enc"
	},
	{
		.name = "Firmware Updater NS_BL2U",
		.uuid = UUID_TRUSTED_UPDATE_FIRMWARE_NS_BL2U,
		.cmdline_name = "fwu",
		.cmdline_enc_name = "fwu-enc"
	},
	{
		.name = "Non-Trusted Firmware Updater certificate",
		.uuid = UUID_TRUSTED_FWU_CERT,
		.cmdline_name = "fwu-cert",
		.cmdline_enc_name = "fwu-cert-enc"
	},
	{
		.name = "Trusted Boot Firmware BL2",
		.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
		.cmdline_name = "tb-fw",
		.cmdline_enc_name = "tb-fw-enc"
	},
	{
		.name = "SCP Firmware SCP_BL2",
		.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
		.cmdline_name = "scp-fw",
		.cmdline_enc_name = "scp-fw-enc"
	},
	{
		.name = "EL3 Runtime Firmware BL31",
		.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
		.cmdline_name = "soc-fw",
		.cmdline_enc_name = "soc-fw-enc"
	},
	{
		.name = "Secure Payload BL32 (Trusted OS)",
		.uuid = UUID_SECURE_PAYLOAD_BL32,
		.cmdline_name = "tos-fw",
		.cmdline_enc_name = "tos-fw-enc"
	},
	{
		.name = "Secure Payload BL32 Extra1 (Trusted OS Extra1)",
		.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA1,
		.cmdline_name = "tos-fw-extra1",
		.cmdline_enc_name = "tos-fw-extra1-enc"
	},
	{
		.name = "Secure Payload BL32 Extra2 (Trusted OS Extra2)",
		.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA2,
		.cmdline_name = "tos-fw-extra2",
		.cmdline_enc_name = "tos-fw-extra2-enc"
	},
	{
		.name = "Non-Trusted Firmware BL33",
		.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
		.cmdline_name = "nt-fw",
		.cmdline_enc_name = "nt-fw-enc"
	},
	/* Dynamic Configs */
	{
		.name = "HW_CONFIG",
		.uuid = UUID_HW_CONFIG,
		.cmdline_name = "hw-config",
		.cmdline_enc_name = "hw-config-enc"
	},
	{
		.name = "TB_FW_CONFIG",
		.uuid = UUID_TB_FW_CONFIG,
		.cmdline_name = "tb-fw-config",
		.cmdline_enc_name = "tb-fw-config-enc"
	},
	{
		.name = "SOC_FW_CONFIG",
		.uuid = UUID_SOC_FW_CONFIG,
		.cmdline_name = "soc-fw-config",
		.cmdline_enc_name = "soc-fw-config-enc"
	},
	{
		.name = "TOS_FW_CONFIG",
		.uuid = UUID_TOS_FW_CONFIG,
		.cmdline_name = "tos-fw-config",
		.cmdline_enc_name = "tos-fw-config-enc"
	},
	{
		.name = "NT_FW_CONFIG",
		.uuid = UUID_NT_FW_CONFIG,
		.cmdline_name = "nt-fw-config",
		.cmdline_enc_name = "nt-fw-config-enc"
	},
	/* Key Certificates */
	{
		.name = "Root Of Trust key certificate",
		.uuid = UUID_ROT_KEY_CERT,
		.cmdline_name = "rot-cert",
		.cmdline_enc_name = "rot-cert-enc"
	},
	{
		.name = "Trusted key certificate",
		.uuid = UUID_TRUSTED_KEY_CERT,
		.cmdline_name = "trusted-key-cert",
		.cmdline_enc_name = "trusted-key-cert-enc"
	},
	{
		.name = "SCP Firmware key certificate",
		.uuid = UUID_SCP_FW_KEY_CERT,
		.cmdline_name = "scp-fw-key-cert",
		.cmdline_enc_name = "scp-fw-key-cert-enc"
	},
	{
		.name = "SoC Firmware key certificate",
		.uuid = UUID_SOC_FW_KEY_CERT,
		.cmdline_name = "soc-fw-key-cert",
		.cmdline_enc_name = "soc-fw-key-cert-enc"
	},
	{
		.name = "Trusted OS Firmware key certificate",
		.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
		.cmdline_name = "tos-fw-key-cert",
		.cmdline_enc_name = "tos-fw-key-cert-enc"
	},
	{
		.name = "Non-Trusted Firmware key certificate",
		.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
		.cmdline_name = "nt-fw-key-cert",
		.cmdline_enc_name = "nt-fw-key-cert-enc"
	},

	/* Content certificates */
	{
		.name = "Trusted Boot Firmware BL2 certificate",
		.uuid = UUID_TRUSTED_BOOT_FW_CERT,
		.cmdline_name = "tb-fw-cert",
		.cmdline_enc_name = "tb-fw-cert-enc"
	},
	{
		.name = "SCP Firmware content certificate",
		.uuid = UUID_SCP_FW_CONTENT_CERT,
		.cmdline_name = "scp-fw-cert",
		.cmdline_enc_name = "scp-fw-cert-enc"
	},
	{
		.name = "SoC Firmware content certificate",
		.uuid = UUID_SOC_FW_CONTENT_CERT,
		.cmdline_name = "soc-fw-cert",
		.cmdline_enc_name = "soc-fw-cert-enc"
	},
	{
		.name = "Trusted OS Firmware content certificate",
		.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
		.cmdline_name = "tos-fw-cert",
		.cmdline_enc_name = "tos-fw-cert-enc"
	},
	{
		.name = "Non-Trusted Firmware content certificate",
		.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
		.cmdline_name = "nt-fw-cert",
		.cmdline_enc_name = "nt-fw-cert-enc"
	},
	{
		.name = NULL,
		.uuid = { {0} },
		.cmdline_name = NULL,
		.cmdline_enc_name = NULL,
	}
};
