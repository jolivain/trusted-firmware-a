/*
 * Copyright (c) 2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <libfdt.h>
#include <common/runtime_svc.h>
#include <smccc_helpers.h>

#include <drivers/arm/gicv3.h>
#include <drivers/arm/gic_common.h>
#include <platform_def.h>
#include <plat/common/platform.h>

static int platform_version_major;
static int platform_version_minor;

#define SMC_FASTCALL       0x80000000
#define SMC64_FUNCTION     (SMC_FASTCALL   | 0x40000000)
#define SIP_FUNCTION       (SMC64_FUNCTION | 0x02000000)
#define SIP_FUNCTION_ID(n) (SIP_FUNCTION   | (n))

#define SIP_SVC_VERSION  SIP_FUNCTION_ID(1)
#define SIP_SVC_GET_GICD SIP_FUNCTION_ID(2)
#define SIP_SVC_GET_GICR SIP_FUNCTION_ID(3)

static const interrupt_prop_t qemu_interrupt_props[] = {
	PLATFORM_G1S_PROPS(INTR_GROUP1S),
	PLATFORM_G0_PROPS(INTR_GROUP0)
};

static uintptr_t qemu_rdistif_base_addrs[PLATFORM_CORE_COUNT];

static unsigned int qemu_mpidr_to_core_pos(unsigned long mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr(mpidr);
}

static gicv3_driver_data_t sbsa_gicv3_driver_data = {
	/* we set those two values for compatibility with older QEMU */
	.gicd_base = GICD_BASE,
	.gicr_base = GICR_BASE,
	.interrupt_props = qemu_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(qemu_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = qemu_rdistif_base_addrs,
	.mpidr_to_core_pos = qemu_mpidr_to_core_pos
};

void read_gic(void *dtb)
{
	int node;
	const fdt32_t *data;

	/*
	 * QEMU gives us this DeviceTree node:
	 *
	 * intc@40060000 {
	 *	#address-cells = <0x02>;
	 *	#interrupt-cells = <0x03>;
	 *	#size-cells = <0x02>;
	 *	compatible = "arm,gic-v3";
	 *	interrupt-controller;
	 *	interrupts = <0x01 0x09 0x04>;
	 *	phandle = <0x8000>;
	 *	ranges;
	 *	reg = <0x00 0x40060000 0x00 0x10000 0x00 0x40080000 0x00 0x4000000>;
	};
	 */
	node = fdt_path_offset(dtb, "/intc");
	if (node >= 0) {
		data = fdt_getprop(dtb, node, "reg", NULL);
		if (data != NULL) {
			INFO("GICD base = 0x%x\n", fdt32_to_cpu(*(data + 1)));
			INFO("GICD size = 0x%x\n", fdt32_to_cpu(*(data + 3)));
			INFO("GICR base = 0x%x\n", fdt32_to_cpu(*(data + 5)));
			INFO("GICR size = 0x%x\n", fdt32_to_cpu(*(data + 7)));

			sbsa_gicv3_driver_data.gicd_base = fdt32_to_cpu(*(data + 1));
			sbsa_gicv3_driver_data.gicr_base = fdt32_to_cpu(*(data + 5));
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

	/* on platform version 0.0 there is no data to read from */
	if (!((platform_version_major == 0) & (platform_version_minor == 0))) {
		read_gic(dtb);
	}
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

	case SIP_SVC_GET_GICD:
		SMC_RET2(handle, NULL, sbsa_gicv3_driver_data.gicd_base);

	case SIP_SVC_GET_GICR:
		SMC_RET2(handle, NULL, sbsa_gicv3_driver_data.gicr_base);

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

int sbsa_sip_smc_setup(void)
{
	INFO("SMC setup done. Version call = 0x%x\n", SIP_SVC_VERSION);
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

void plat_qemu_gic_init(void)
{
	gicv3_driver_init(&sbsa_gicv3_driver_data);
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void qemu_pwr_gic_on_finish(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void qemu_pwr_gic_off(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
	gicv3_rdistif_off(plat_my_core_pos());
}
