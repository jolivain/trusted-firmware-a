/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DICE_PROT_ENV_H
#define DICE_PROT_ENV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* TODO: Due to size related defines, might move them to a separate header */
#include <measured_boot.h>

#define DPE_INVALID_ID	UINT32_MAX

struct dpe_metadata {
	unsigned int id;
	uint8_t signer_id[SIGNER_ID_MAX_SIZE];
	size_t  signer_id_size;
	uint8_t version[VERSION_MAX_SIZE];
	size_t  version_size;
	uint8_t sw_type[SW_TYPE_MAX_SIZE];
	size_t  sw_type_size;
	bool allow_child_to_derive;
	bool retain_parent_context;
	bool create_certificate;
};

void dpe_init(void);
struct dpe_metadata *plat_dpe_get_metadata(void);
void plat_dpe_share_context_handle(int *ctx_handle);
int dpe_measure_and_record(uintptr_t data_base, uint32_t data_size,
			   uint32_t data_id);

#endif /* DICE_PROT_ENV_H */
