/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_HW_CONFIG_HELPERS_H
#define FCONF_HW_CONFIG_HELPERS_H


uint64_t construct_input_addr(const uint32_t *value, int child_addr_size);
uint64_t construct_output_addr(const uint32_t *value, int child_addr_size, int parent_addr_size);
int find_mapping(const uint32_t *value, int child_addr_size, int parent_addr_size,
			uint64_t base_address, uint64_t *translated_addr);
int search_all_xlat_entries(const uint32_t *value, int child_addr_size, int parent_addr_size,
	uint64_t base_address, uint64_t *translated_addr, int xlat_cells, int num_entries);
uint64_t translate_address(const void *dtb, int bus_node, uint64_t base_address);


#endif /* FCONF_HW_CONFIG_HELPERS_H */
