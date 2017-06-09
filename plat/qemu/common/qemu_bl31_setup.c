/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <plat/common/platform.h>

#include "qemu_private.h"

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL3-1 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*******************************************************************************
 * Perform any BL3-1 early platform setup.  Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & EL3 in BL1) before
 * they are lost (potentially). This needs to be done before the MMU is
 * initialized so that the memory layout can be used while creating page
 * tables. BL2 has flushed this information to memory, so we are guaranteed
 * to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	qemu_console_init();

	/*
	 * Check params passed from BL2
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;

	assert(params_from_bl2);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (!bl33_image_ep_info.pc)
		panic();
}

void bl31_plat_arch_setup(void)
{
	qemu_configure_mmu_el3(BL31_BASE, (BL31_END - BL31_BASE),
			      BL_CODE_BASE, BL_CODE_END,
			      BL_RO_DATA_BASE, BL_RO_DATA_END,
			      BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END);
}

void bl31_platform_setup(void)
{
	plat_qemu_gic_init();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image
 * for the security state specified. BL3-3 corresponds to the non-secure
 * image type while BL3-2 corresponds to the secure image type. A NULL
 * pointer is returned if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;
	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

#if ENABLE_SPM && SPM_MM

/* Region equivalent to MAP_DEVICE1 suitable for mapping at EL0 */
#define MAP_DEVICE1_EL0	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE | MT_USER)

const mmap_region_t plat_qemu_secure_partition_mmap[] = {
	MAP_DEVICE1_EL0, /* for the UART */
	QEMU_SP_IMAGE_MMAP,
	QEMU_SPM_BUF_EL0_MMAP,
	QEMU_SP_IMAGE_NS_BUF_MMAP,
	QEMU_SP_IMAGE_RW_MMAP,
	{0}
};

/*
 * Boot information passed to a secure partition during initialisation. Linear
 * indices in MP information will be filled at runtime.
 */
static secure_partition_mp_info_t sp_mp_info[] = {
	[0] = {0x80000000, 0},
	[1] = {0x80000001, 0},
	[2] = {0x80000002, 0},
	[3] = {0x80000003, 0},
	[4] = {0x80000100, 0},
	[5] = {0x80000101, 0},
	[6] = {0x80000102, 0},
	[7] = {0x80000103, 0},
};

const secure_partition_boot_info_t plat_qemu_secure_partition_boot_info = {
	.h.type              = PARAM_SP_IMAGE_BOOT_INFO,
	.h.version           = VERSION_1,
	.h.size              = sizeof(secure_partition_boot_info_t),
	.h.attr              = 0,
	.sp_mem_base         = QEMU_SP_IMAGE_BASE,
	.sp_mem_limit        = QEMU_SP_IMAGE_LIMIT,
	.sp_image_base       = QEMU_SP_IMAGE_BASE,
	.sp_stack_base       = PLAT_SP_IMAGE_STACK_BASE,
	.sp_heap_base        = QEMU_SP_IMAGE_HEAP_BASE,
	.sp_ns_comm_buf_base = QEMU_SP_IMAGE_NS_BUF_BASE,
	.sp_shared_buf_base  = PLAT_SPM_BUF_BASE,
	.sp_image_size       = QEMU_SP_IMAGE_SIZE,
	.sp_pcpu_stack_size  = PLAT_SP_IMAGE_STACK_PCPU_SIZE,
	.sp_heap_size        = QEMU_SP_IMAGE_HEAP_SIZE,
	.sp_ns_comm_buf_size = QEMU_SP_IMAGE_NS_BUF_SIZE,
	.sp_shared_buf_size  = PLAT_SPM_BUF_SIZE,
	.num_sp_mem_regions  = QEMU_SP_IMAGE_NUM_MEM_REGIONS,
	.num_cpus            = PLATFORM_CORE_COUNT,
	.mp_info             = &sp_mp_info[0],
};

const mmap_region_t *plat_get_secure_partition_mmap(void *cookie)
{
	return plat_qemu_secure_partition_mmap;
}

const secure_partition_boot_info_t *plat_get_secure_partition_boot_info(
	void *cookie)
{
	return &plat_qemu_secure_partition_boot_info;
}

/*
 * Enumeration of priority levels on QEMU platforms.
 */
ehf_pri_desc_t qemu_exceptions[] = {
#if RAS_EXTENSION
	/* RAS Priority */
	EHF_PRI_DESC(QEMU_PRI_BITS, PLAT_RAS_PRI),
#endif

#if SDEI_SUPPORT
	/* Critical priority SDEI */
	EHF_PRI_DESC(QEMU_PRI_BITS, PLAT_SDEI_CRITICAL_PRI),

	/* Normal priority SDEI */
	EHF_PRI_DESC(QEMU_PRI_BITS, PLAT_SDEI_NORMAL_PRI),
#endif

	EHF_PRI_DESC(QEMU_PRI_BITS, PLAT_SP_PRI),
};

/* Plug in QEMU exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(qemu_exceptions, ARRAY_SIZE(qemu_exceptions), QEMU_PRI_BITS);

#endif /* ENABLE_SPM && SPM_MM */

