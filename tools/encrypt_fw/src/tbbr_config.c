/*
 * Copyright (c) 2019, Linaro Limited
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tbbr_config.h"

/*
 * Firmware table which can be encrypted.
 */
static fw_t tbb_fws[] = {
	{
		.opt = "scp-fwu-cfg",
		.help_msg = "SCP Firmware Updater Cfg. (in)",
	},
	{
		.opt = "scp-fwu-cfg-enc",
		.help_msg = "Encrypted SCP Firmware Updater Cfg. (out)",
	},
	{
		.opt = "ap-fwu-cfg",
		.help_msg = "AP Firmware Updater Configuration (in)",
	},
	{
		.opt = "ap-fwu-cfg-enc",
		.help_msg = "Encrypted AP Firmware Updater Configuration (out)",
	},
	{
		.opt = "fwu",
		.help_msg = "Firmware Updater (in)",
	},
	{
		.opt = "fwu-enc",
		.help_msg = "Encrypted Firmware Updater (out)",
	},
	{
		.opt = "fwu-cert",
		.help_msg = "Non-Trusted Firmware Updater cert (in)",
	},
	{
		.opt = "fwu-cert-enc",
		.help_msg = "Encrypted Non-Trusted Firmware Updater cert (out)",
	},
	{
		.opt = "tb-fw",
		.help_msg = "Trusted Boot Firmware BL2 (in)",
	},
	{
		.opt = "tb-fw-enc",
		.help_msg = "Encrypted Trusted Boot Firmware BL2 (out)",
	},
	{
		.opt = "scp-fw",
		.help_msg = "SCP Firmware SCP_BL2 (in)",
	},
	{
		.opt = "scp-fw-enc",
		.help_msg = "Encrypted SCP Firmware SCP_BL2 (out)",
	},
	{
		.opt = "soc-fw",
		.help_msg = "EL3 Runtime Firmware BL31 (in)",
	},
	{
		.opt = "soc-fw-enc",
		.help_msg = "Encrypted EL3 Runtime Firmware BL31 (out)",
	},
	{
		.opt = "tos-fw",
		.help_msg = "Secure Payload BL32 (Trusted OS) (in)",
	},
	{
		.opt = "tos-fw-enc",
		.help_msg = "Encrypted Secure Payload BL32 (Trusted OS) (out)",
	},
	{
		.opt = "tos-fw-extra1",
		.help_msg = "Secure Payload BL32 Extra1 (in)",
	},
	{
		.opt = "tos-fw-extra1-enc",
		.help_msg = "Encrypted Secure Payload BL32 Extra1 (out)",
	},
	{
		.opt = "tos-fw-extra2",
		.help_msg = "Secure Payload BL32 Extra2 (in)",
	},
	{
		.opt = "tos-fw-extra2-enc",
		.help_msg = "Encrypted Secure Payload BL32 Extra2 (out)",
	},
	{
		.opt = "nt-fw",
		.help_msg = "Non-Trusted Firmware BL33 (in)",
	},
	{
		.opt = "nt-fw-enc",
		.help_msg = "Encrypted Non-Trusted Firmwre BL33 (out)",
	},
	/* Dynamic Configs */
	{
		.opt = "hw-config",
		.help_msg = "HW_CONFIG (in)",
	},
	{
		.opt = "hw-config-enc",
		.help_msg = "Encrypted HW_CONFIG (out)",
	},
	{
		.opt = "tb-fw-config",
		.help_msg = "TB_FW_CONFIG (in)",
	},
	{
		.opt = "tb-fw-config-enc",
		.help_msg = "Encrypted TB_FW_CONFIG (out)",
	},
	{
		.opt = "soc-fw-config",
		.help_msg = "SOC_FW_CONFIG (in)",
	},
	{
		.opt = "soc-fw-config-enc",
		.help_msg = "Encrypted SOC_FW_CONFIG (out)",
	},
	{
		.opt = "tos-fw-config",
		.help_msg = "TOS_FW_CONFIG (in)",
	},
	{
		.opt = "tos-fw-config-enc",
		.help_msg = "Encrypted TOS_FW_CONFIG (out)",
	},
	{
		.opt = "nt-fw-config",
		.help_msg = "NT_FW_CONFIG (in)",
	},
	{
		.opt = "nt-fw-config-enc",
		.help_msg = "Encrypted NT_FW_CONFIG (out)",
	},
	/* Key Certificates */
	{
		.opt = "rot-cert",
		.help_msg = "Root Of Trust key certificate (in)",
	},
	{
		.opt = "rot-cert-enc",
		.help_msg = "Encrypted Root Of Trust key certificate (out)",
	},
	{
		.opt = "trusted-key-cert",
		.help_msg = "Trusted key certificate (in)",
	},
	{
		.opt = "trusted-key-cert-enc",
		.help_msg = "Encrypted Trusted key certificate (out)",
	},
	{
		.opt = "scp-fw-key-cert",
		.help_msg = "SCP Firmware key certificate (in)",
	},
	{
		.opt = "scp-fw-key-cert-enc",
		.help_msg = "Encrypted SCP Firmware key certificate (out)",
	},
	{
		.opt = "soc-fw-key-cert",
		.help_msg = "SoC Firmware key certificate (in)",
	},
	{
		.opt = "soc-fw-key-cert-enc",
		.help_msg = "Encrypted SoC Firmware key certificate (out)",
	},
	{
		.opt = "tos-fw-key-cert",
		.help_msg = "Trusted OS Firmware key certificate (in)",
	},
	{
		.opt = "tos-fw-key-cert-enc",
		.help_msg = "Encrypted Trusted OS Firmware key cert (out)",
	},
	{
		.opt = "nt-fw-key-cert",
		.help_msg = "Non-Trusted Firmware key certificate (in)",
	},
	{
		.opt = "nt-fw-key-cert-enc",
		.help_msg = "Encrypted Non-Trusted Firmware key cert (out)",
	},

	/* Content certificates */
	{
		.opt = "tb-fw-cert",
		.help_msg = "Trusted Boot Firmware BL2 certificate (in)",
	},
	{
		.opt = "tb-fw-cert-enc",
		.help_msg = "Encrypted Trusted Boot Firmware BL2 cert (out)",
	},
	{
		.opt = "scp-fw-cert",
		.help_msg = "SCP Firmware content certificate (in)",
	},
	{
		.opt = "scp-fw-cert-enc",
		.help_msg = "Encrypted SCP Firmware content certificate (out)",
	},
	{
		.opt = "soc-fw-cert",
		.help_msg = "SoC Firmware content certificate (in)",
	},
	{
		.opt = "soc-fw-cert-enc",
		.help_msg = "Encrypted SoC Firmware content certificate (out)",
	},
	{
		.opt = "tos-fw-cert",
		.help_msg = "Trusted OS Firmware content cert (in)",
	},
	{
		.opt = "tos-fw-cert-enc",
		.help_msg = "Encrypted Trusted OS Firmware content cert (out)",
	},
	{
		.opt = "nt-fw-cert",
		.help_msg = "Non-Trusted Firmware content cert (in)",
	},
	{
		.opt = "nt-fw-cert-enc",
		.help_msg = "Encrypted Non-Trusted Firmware content cert (out)",
	},
};

REGISTER_FWS(tbb_fws);
