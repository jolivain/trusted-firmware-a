/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <fconf_hw_config_helpers.h>
#include <libfdt.h>
#include <plat/common/platform.h>

/* address mapping needs to be done recursively starting from current(child)
 * node to root node through all intermediate parent nodes.
 * Sample device tree is shown here:

smb@0,0 {
	compatible = "simple-bus";

	#address-cells = <2>;
	#size-cells = <1>;
	ranges = <0 0 0 0x08000000 0x04000000>,
		 <1 0 0 0x14000000 0x04000000>,
		 <2 0 0 0x18000000 0x04000000>,
		 <3 0 0 0x1c000000 0x04000000>,
		 <4 0 0 0x0c000000 0x04000000>,
		 <5 0 0 0x10000000 0x04000000>;

	motherboard {
		arm,v2m-memory-map = "rs1";
		compatible = "arm,vexpress,v2m-p1", "simple-bus";
		#address-cells = <2>;
		#size-cells = <1>;
		ranges;

		iofpga@3,00000000 {
			compatible = "arm,amba-bus", "simple-bus";
			#address-cells = <1>;
			#size-cells = <1>;
			ranges = <0 3 0 0x200000>;
		};
	};
};

 * As seen above, there are three levels of address translations needed. An empty ranges
 * property denotes identity mapping (as seen in motherboard node).
 * Each ranges property can map a set of child addresses to parent bus. Hence there
 * can be more than 1 (translation) entry in the ranges property as seen in the
 * smb node which has 6 translation entries.
*/

uint64_t construct_input_addr(const uint32_t *value, int child_addr_size)
{
	uint32_t high, low;

	high = fdt32_to_cpu(value[0]);

	/* Number of cells used to specify address can be upto a max of 2 */
	if (child_addr_size == 1) {
		return (uint64_t)high;
	} else if (child_addr_size == 2) {
		low = fdt32_to_cpu(value[1]);
		return ((uint64_t)high << 32) | low;
	} else {
		ERROR("DT: Property address-size cannot be greater than 2");
		return 0;
	}
}

uint64_t construct_output_addr(const uint32_t *value, int child_addr_size, int parent_addr_size)
{
	int parent_addr_index = child_addr_size;
	uint32_t high, low;

	high = fdt32_to_cpu(value[parent_addr_index]);

	/* Number of cells used to specify address can be upto a max of 2 */
	if (parent_addr_size == 1) {
		return (uint64_t)high;
	} else if (parent_addr_size == 2) {
		low = fdt32_to_cpu(value[parent_addr_index+1]);
		return ((uint64_t)high << 32) | low;
	} else {
		ERROR("DT: Property address-size cannot be greater than 2");
		return 0;
	}

}

int find_mapping(const uint32_t *value, int child_addr_size, int parent_addr_size,
			uint64_t base_address, uint64_t *translated_addr)
{
	uint64_t local_address, parent_address;

	local_address = construct_input_addr(value, child_addr_size);
	parent_address = construct_output_addr(value, child_addr_size, parent_addr_size);
	VERBOSE("DT: Address %llx mapped to %llx\n", local_address, parent_address);

	if (local_address == base_address) {
		*translated_addr = parent_address;
		VERBOSE("DT: child address %llx mapped to %llx in parent bus\n",
				local_address, parent_address);
		return 0;
	}
	return -1;
}

int search_all_xlat_entries(const uint32_t *value, int child_addr_size, int parent_addr_size,
	uint64_t base_address, uint64_t *translated_addr, int xlat_cells, int num_entries)
{
	int found;
	const uint32_t *next_entry = value;

	for (int i = 0; i < num_entries; i++) {
		VERBOSE("DT: Address translation entry: %d\n", i);
		found = find_mapping(next_entry, child_addr_size, parent_addr_size,
				base_address, translated_addr);
		if (found == 0) {
			return 0;
		}
		next_entry = next_entry + xlat_cells;
	}
	return -1;
}


/* Recursive implementation */
uint64_t translate_address(const void *dtb, int bus_node, uint64_t base_address)
{
	int length, parent_node, found, nxlat_entries;
	static int lookup_lvl;
	const char *node_name;
	uint64_t global_address;

	node_name = fdt_get_name(dtb, bus_node, NULL);
	parent_node = fdt_parent_offset(dtb, bus_node);

	/* Read the ranges property */
	const uint32_t *ptr = fdt_getprop(dtb, bus_node, "ranges", &length);

	if (ptr == NULL) {
		if (bus_node == 0) {
			/* root node doesn't have range property as the addresses
			 * are in CPU address space.
			 */
			return base_address;
		}
		ERROR("DT: Couldn't find ranges property in node %s\n", node_name);
		panic();
	} else if (length == 0) {
		/* empty ranges indicates identity map to parent bus */
		lookup_lvl++;
		return translate_address(dtb, parent_node, base_address);
	} else {

		VERBOSE("DT: Translation lookup in node %s at level: %d\n", node_name, lookup_lvl);

		/* The number of cells in one translation entry in ranges is the sum of the
		 * following values:
		 * self#address-cells + parent#address-cells + self#size-cells
		 * For example, the iofpga ranges property has one translation entry with 4 cells
		 * They represent iofpga#addr-cells + motherboard#addr-cells + iofpga#size-cells
		 *              = 1                 + 2                      + 1
		 */
		int self_addr_cells, parent_addr_cells, self_size_cells, ncells_xlat;

		self_addr_cells = fdt_address_cells(dtb, bus_node);
		self_size_cells = fdt_size_cells(dtb, bus_node);
		parent_addr_cells = fdt_address_cells(dtb, parent_node);

		/* Number of cells per translation entry i.e., mapping */
		ncells_xlat = self_addr_cells + parent_addr_cells + self_size_cells;

		assert(ncells_xlat > 0);

		/* Number of translations(mappings) specified in the current ranges property.
		 * Note that length represents number of bytes and each cell is 4 bytes long.
		 */
		nxlat_entries = (length/4)/ncells_xlat;

		assert(nxlat_entries > 0);

		found = search_all_xlat_entries(ptr, self_addr_cells, parent_addr_cells,
					base_address, &global_address, ncells_xlat, nxlat_entries);
		if (found < 0) {
			ERROR("DT: No translation found for address %llx in node %s\n",
				base_address, node_name);
			panic();
		}
		/* Translate the local device address recursively */
		lookup_lvl++;
		return translate_address(dtb, parent_node, global_address);
	}
}
