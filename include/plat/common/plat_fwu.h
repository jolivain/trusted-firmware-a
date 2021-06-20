/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_FWU_H
#define PLAT_FWU_H

/*
 * Function to set the source address of metadata image in the
 * IO policy.
 */
int plat_set_fwu_metadata_image_source(unsigned int image_id);

/*
 * Function to set the source addresses of updated images in
 * the IO policy.
 */
void plat_set_fwu_images_source(uint32_t active_fw_bank_idx);

#endif /* PLAT_FWU_H */
