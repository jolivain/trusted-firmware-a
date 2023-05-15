/*
 * Copyright (c) 2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/runtime_svc.h>
#include <libfdt.h>
#include <smccc_helpers.h>

#include <plat/common/platform.h>

static int platform_version_major;
static int platform_version_minor;

#define SMC_FASTCALL       0x80000000
#define SMC64_FUNCTION     (SMC_FASTCALL   | 0x40000000)
#define SIP_FUNCTION       (SMC64_FUNCTION | 0x02000000)
#define SIP_FUNCTION_ID(n) (SIP_FUNCTION   | (n))

#define SIP_SVC_VERSION  SIP_FUNCTION_ID(1)

#define SIP_SVC_GET_GIC  SIP_FUNCTION_ID(100)

void sbsa_set_gic_bases(const uintptr_t gicd_base, const uintptr_t gicr_base);
uintptr_t sbsa_get_gicd(void);
uintptr_t sbsa_get_gicr(void);

void read_platform_config_from_dt(void *dtb)
{
	int node;
	const fdt64_t *data;

	/*
	 * QEMU gives us this DeviceTree node:
	 *
	 * intc {
		reg = < 0x00 0x40060000 0x00 0x10000
			0x00 0x40080000 0x00 0x4000000>;
	};
	 */
	node = fdt_path_offset(dtb, "/intc");
	if (node >= 0) {
		data = fdt_getprop(dtb, node, "reg", NULL);
		if (data != NULL) {
			INFO("GICD base = 0x%lx\n", fdt64_to_cpu(*(data + 0)));
			INFO("GICD size = 0x%lx\n", fdt64_to_cpu(*(data + 1)));
			INFO("GICR base = 0x%lx\n", fdt64_to_cpu(*(data + 2)));
			INFO("GICR size = 0x%lx\n", fdt64_to_cpu(*(data + 3)));

			sbsa_set_gic_bases(fdt64_to_cpu(*(data + 0)), fdt64_to_cpu(*(data + 2)));
		}
	}
}

void read_platform_version(void *dtb)
{
	int node;

	node = fdt_path_offset(dtb, "/");
	if (node >= 0) {
		platform_version_major = fdt32_ld(fdt_getprop(dtb, node,
							      "machine-version-major", NULL));
		platform_version_minor = fdt32_ld(fdt_getprop(dtb, node,
							      "machine-version-minor", NULL));
	}
}

void sip_svc_init(void)
{
	/* Read DeviceTree data before MMU is enabled */

	void *dtb = (void *)(uintptr_t)ARM_PRELOADED_DTB_BASE;
	int err;

	err = fdt_open_into(dtb, dtb, PLAT_QEMU_DT_MAX_SIZE);
	if (err < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", dtb, err);
	}

	err = fdt_check_header(dtb);
	if (err < 0) {
		WARN("Invalid DTB file passed\n");
	}

	read_platform_version(dtb);
	INFO("Platform version: %d.%d\n", platform_version_major, platform_version_minor);

	read_platform_config_from_dt(dtb);
}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uintptr_t sbsa_sip_smc_handler(uint32_t smc_fid,
			       u_register_t x1,
			       u_register_t x2,
			       u_register_t x3,
			       u_register_t x4,
			       void *cookie,
			       void *handle,
			       u_register_t flags)
{
	uint32_t ns;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		ERROR("%s: wrong world SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	switch (smc_fid) {
	case SIP_SVC_VERSION:
		INFO("Platform version requested\n");
		SMC_RET3(handle, NULL, platform_version_major, platform_version_minor);

	case SIP_SVC_GET_GIC:
		SMC_RET3(handle, NULL, sbsa_get_gicd(), sbsa_get_gicr());

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

int sbsa_sip_smc_setup(void)
{
	return 0;
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	sbsa_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	sbsa_sip_smc_setup,
	sbsa_sip_smc_handler
);
