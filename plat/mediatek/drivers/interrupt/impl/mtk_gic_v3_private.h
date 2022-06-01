/*
 * Copyright (C) 2021 Mediatek Inc.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MTK_GIC_V3_PRIVATE_H
#define MTK_GIC_V3_PRIVATE_H

#include <lib/utils_def.h>

#if GIC_DEBUG
#define GIC_DPRINT(...) NOTICE("[GIC Dump] " __VA_ARGS__)
#else
#define GIC_DPRINT(...)
#endif

#ifndef MAX_GIC_NR
#define MAX_GIC_NR			(1)
#endif

#ifndef MAX_IRQ_NR
#define MAX_IRQ_NR			(TOTAL_SPI_INTR_NUM + TOTAL_PCPU_INTR_NUM)
#endif

#define MAX_RDIST_NR			(64)
#define SGI_MASK 0xffff

struct gic_chip_data {
	unsigned int saved_enable[DIV_ROUND_UP_2EVAL(MAX_IRQ_NR, 32)];
	unsigned int saved_conf[DIV_ROUND_UP_2EVAL(MAX_IRQ_NR, 16)];
	unsigned int saved_priority[DIV_ROUND_UP_2EVAL(MAX_IRQ_NR, 4)];
	uint64_t saved_target[DIV_ROUND_UP_2EVAL(MAX_IRQ_NR, 1)];
	unsigned int saved_group[DIV_ROUND_UP_2EVAL(MAX_IRQ_NR, 32)];
	unsigned int saved_grpmod[DIV_ROUND_UP_2EVAL(MAX_IRQ_NR, 32)];
	unsigned int rdist_base[MAX_RDIST_NR];
	unsigned int saved_spi_pending[DIV_ROUND_UP_2EVAL(MAX_IRQ_NR, 32)];
	unsigned int saved_active_sel;
	unsigned int saved_sgi[PLATFORM_CORE_COUNT];
};

#endif

