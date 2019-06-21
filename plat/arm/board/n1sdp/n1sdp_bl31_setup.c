/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>
#include <n1sdp_def.h>

#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <plat/arm/common/plat_arm.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils.h>

/*
 * Platform information structure stored in non secure SRAM.
 * This structure holds the total DDR memory size which will be
 * used when zeroing out the entire DDR memory before enabling
 * the ECC capability in DMCs.
 */
struct n1sdp_plat_info {
	uint32_t ddr_size_gb;
};

static scmi_channel_plat_info_t n1sdp_scmi_plat_info = {
		.scmi_mbx_mem = N1SDP_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + CSS_SCMI_MHU_DB_REG_OFF,
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
};

scmi_channel_plat_info_t *plat_css_get_scmi_info()
{
	return &n1sdp_scmi_plat_info;
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}

/*
 * N1SDP platform supports RDIMMs with ECC capability. To use the ECC
 * capability, the entire DDR memory space has to be zeroed out before
 * enabling the ECC bits in DMC620. Zeroing out several gigabytes of
 * memory from SCP is quite time consuming so the following function
 * is added to zero out the DDR memory from application processor which is
 * much faster compared to SCP. BL33 binary cannot be copied to DDR memory
 * before enabling ECC so copy_bl33 function is added to copy BL33 binary
 * from IOFPGA-DDR3 memory to main DDR4 memory.
 */

void dmc_ecc_setup(void)
{
	struct n1sdp_plat_info *plat_info;
	uint64_t dram2_size;

	plat_info = (struct n1sdp_plat_info *)N1SDP_INFO_STRUCT_BASE;
	dram2_size = (plat_info->ddr_size_gb * 1024UL * 1024UL * 1024UL) -
			ARM_DRAM1_SIZE;

	INFO("Zeroing DDR memories\n");
	zero_normalmem((void *)ARM_DRAM1_BASE, ARM_DRAM1_SIZE);
	flush_dcache_range(ARM_DRAM1_BASE, ARM_DRAM1_SIZE);
	zero_normalmem((void *)ARM_DRAM2_BASE, dram2_size);
	flush_dcache_range(ARM_DRAM2_BASE, dram2_size);

	INFO("Enabling ECC on DMCs\n");
	mmio_setbits_32(N1SDP_DMC0_ERR0CTLR0_REG, N1SDP_DMC_ERR0CTLR0_ECC_EN);
	mmio_setbits_32(N1SDP_DMC1_ERR0CTLR0_REG, N1SDP_DMC_ERR0CTLR0_ECC_EN);
}

void copy_bl33(void)
{
	uint32_t i;

	INFO("Copying BL33 to DDR memory\n");
	for (i = 0; i < N1SDP_BL33_SIZE; i = i + 8)
		mmio_write_64((N1SDP_BL33_DST_BASE + i),
			mmio_read_64(N1SDP_BL33_SRC_BASE + i));

	for (i = 0; i < N1SDP_BL33_SIZE; i = i + 8) {
		if (mmio_read_64(N1SDP_BL33_SRC_BASE + i) !=
			mmio_read_64(N1SDP_BL33_DST_BASE + i)) {
			INFO("Copy failed!\n");
			assert(false);
		}
	}
}

void bl31_platform_setup(void)
{
	arm_bl31_platform_setup();
	dmc_ecc_setup();
	copy_bl33();
}
