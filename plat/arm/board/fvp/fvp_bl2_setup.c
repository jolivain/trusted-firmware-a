/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/transfer_list.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "fvp_private.h"

static struct transfer_list_header *ns_tl __unused;
static struct transfer_list_header *bl31_tl __unused;

#if TRANSFER_LIST
static struct bl_params next_bl_params;
#endif

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	arm_bl2_early_platform_setup((uintptr_t)arg0, (meminfo_t *)arg1);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

#if TRANSFER_LIST
	ns_tl = transfer_list_init((void *)FW_NS_HANDOFF_BASE, FW_HANDOFF_SIZE);
	assert(ns_tl != NULL);

	bl31_tl = transfer_list_init((void *)FW_BL31_HANDOFF_BASE,
				     FW_HANDOFF_SIZE);
	assert(bl31_tl != NULL);
#endif
	/* Initialize System level generic or SP804 timer */
	fvp_timer_init();
}

/*******************************************************************************
 * This function returns the list of executable images
 ******************************************************************************/
struct bl_params *plat_get_next_bl_params(void)
{
	struct bl_params *arm_bl_params;
	const struct dyn_cfg_dtb_info_t *hw_config_info __unused;
	struct transfer_list_entry *te __unused;
	bl_mem_params_node_t *param_node __unused;
	uint32_t next_exe_img_id  __unused;

	entry_point_info_t *ep __unused;

#if __aarch64__
	/* Get BL31 image node */
	param_node = get_bl_mem_params_node(BL31_IMAGE_ID);
#else /* aarch32 */
	/* Get SP_MIN image node */
	param_node = get_bl_mem_params_node(BL32_IMAGE_ID);
#endif /* __aarch64__ */
	assert(param_node != NULL);

#if TRANSFER_LIST
	arm_bl_params = &next_bl_params;
	SET_PARAM_HEAD(arm_bl_params, PARAM_BL_PARAMS, VERSION_2, 0U);

	arm_bl_params->head = &param_node->params_node_mem;
	arm_bl_params->head->ep_info = &param_node->ep_info;
	arm_bl_params->head->image_id = param_node->image_id;

	ep = transfer_list_set_handoff_args(bl31_tl,
					    arm_bl_params->head->ep_info);
	assert(ep != NULL);

	flush_dcache_range((uintptr_t)arm_bl_params->head->ep_info, sizeof(entry_point_info_t));

	next_exe_img_id = param_node->next_handoff_image_id;

	entry_point_info_t ep_info[4];
	bl_mem_params_node_t *desc_ptr;
	size_t ep_index = 0;

	while (next_exe_img_id != INVALID_IMAGE_ID) {
		desc_ptr = &bl_mem_params_desc_ptr[get_bl_params_node_index(
			next_exe_img_id)];

		ep_info[ep_index] = desc_ptr->ep_info;

		if (next_exe_img_id == BL33_IMAGE_ID) {
			ep = transfer_list_set_handoff_args(
				ns_tl, &ep_info[ep_index++]);
			assert(ep != NULL);
		}

		next_exe_img_id = desc_ptr->next_handoff_image_id;
	}

	te = transfer_list_add(bl31_tl, TL_TAG_EP_INFO,
			       ep_index * sizeof(entry_point_info_t), &ep_info);
	assert(te != NULL);

	flush_dcache_range((uintptr_t)bl31_tl, bl31_tl->size);
	flush_dcache_range((uintptr_t)ns_tl, ns_tl->size);
#else
	arm_bl_params = arm_get_next_bl_params();

#if !RESET_TO_BL2 && !EL3_PAYLOAD_BASE
	const struct dyn_cfg_dtb_info_t *fw_config_info __unused;
	uintptr_t fw_config_base __unused;

	/* Update the next image's ep info with the FW config address */
	fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, FW_CONFIG_ID);
	assert(fw_config_info != NULL);

	fw_config_base = fw_config_info->config_addr;
	assert(fw_config_base != 0UL);

	param_node->ep_info.args.arg1 = (uint32_t)fw_config_base;

	/* Update BL33's ep info with the NS HW config address */
	param_node = get_bl_mem_params_node(BL33_IMAGE_ID);
	assert(param_node != NULL);

	hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
	assert(hw_config_info != NULL);

	param_node->ep_info.args.arg1 = hw_config_info->secondary_config_addr;
#endif /* !RESET_TO_BL2 && !EL3_PAYLOAD_BASE */
#endif /* TRANSFER_LIST */

	return arm_bl_params;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	if (image_id == HW_CONFIG_ID) {
		const struct dyn_cfg_dtb_info_t *hw_config_info;
		struct transfer_list_entry *te __unused;

		bl_mem_params_node_t *param_node =
			get_bl_mem_params_node(image_id);
		assert(param_node != NULL);

		hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
		assert(hw_config_info != NULL);

#if TRANSFER_LIST
		te = transfer_list_find(bl31_tl, TL_TAG_FDT);
		assert(te != NULL);

		/* Refresh the now stale checksum following loading of HW_CONFIG into the TL. */
		transfer_list_update_checksum(bl31_tl);

		/* Update BL31's transfer list checksum after copying the HW_CONFIG into
		 * the preallocated TE. */
		/* Update BL33's ep info with NS HW config address  */
		te = transfer_list_add(ns_tl, TL_TAG_FDT, te->data_size,
				       transfer_list_entry_data(te));
		assert(te != NULL);

#else
#if !RESET_TO_BL2 && !EL3_PAYLOAD_BASE
		memcpy((void *)hw_config_info->secondary_config_addr,
		       (void *)hw_config_info->config_addr,
		       (size_t)param_node->image_info.image_size);

		/*
		 * Ensure HW-config device tree is committed to memory, as the HW-Config
		 * might be used without cache and MMU enabled at BL33.
		 */
		flush_dcache_range(hw_config_info->secondary_config_addr,
				   param_node->image_info.image_size);
#endif /* !RESET_TO_BL2 && !EL3_PAYLOAD_BASE */
#endif /* TRANSFER_LIST */
	}

	return arm_bl2_plat_handle_post_image_load(image_id);
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
#if TRANSFER_LIST
	if (image_id == HW_CONFIG_ID) {
		struct transfer_list_entry *te __unused;

		bl_mem_params_node_t *param_node =
			get_bl_mem_params_node(image_id);
		assert(param_node != NULL);

		/* The HW_CONFIG needs to be authenticated via the normal loading
		 * mechanism. Pre-allocate a TE for the configuration and update the
		 * load information so the configuration is loaded directly into the TE. */
		te = transfer_list_add(bl31_tl, TL_TAG_FDT,
				       param_node->image_info.image_max_size,
				       NULL);
		assert(te != NULL);
		param_node->image_info.image_base =
			(uintptr_t)transfer_list_entry_data(te);
	}
#endif /* TRANSFER_LIST */
	return 0;
}
