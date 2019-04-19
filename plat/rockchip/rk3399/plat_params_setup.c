/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <libfdt.h>
#include <plat_private.h>

void plat_rockchip_dt_process_fdt_uart(void *fdt)
{
	const char *path_name = "/chosen";
	const char *prop_name = "stdout-path";
	int node_offset;
	int stdout_path_len;
	const char *stdout_path;
	char serial_char;
	int serial_no;
	uint32_t uart_base;

	node_offset = fdt_path_offset(fdt, path_name);
	if (node_offset < 0)
		return;

	stdout_path = fdt_getprop(fdt, node_offset, prop_name,
				  &stdout_path_len);
	if (stdout_path == NULL)
		return;

	/*
	 * We expect something like:
	 *   "serial0:...""
	 */
	if (strncmp("serial", stdout_path, 6) != 0)
		return;

	serial_char = stdout_path[6];
	serial_no = serial_char - '0';

	switch (serial_no) {
	case 0:
		uart_base = UART0_BASE;
		break;
	case 1:
		uart_base = UART1_BASE;
		break;
	case 2:
		uart_base = UART2_BASE;
		break;
	case 3:
		uart_base = UART3_BASE;
		break;
	default:
		return;
	}

	rockchip_set_uart_base(uart_base);
}

