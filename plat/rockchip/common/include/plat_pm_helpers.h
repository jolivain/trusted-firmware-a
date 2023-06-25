/*
 * Copyright (C) 2023, Rockchip Electronics Co., Ltd.
 * All rights reserved.
 */

#ifndef PLAT_PM_HELPERS_H
#define PLAT_PM_HELPERS_H

#include <stdint.h>

#define REG_REGION(_start, _end, _stride, _base, _wmsk)	\
{							\
	.start = (_base) + (_start),			\
	.end   = (_base) + (_end),			\
	.stride   = _stride,				\
	.wmsk  = _wmsk					\
}

struct reg_region {
	uint32_t start;
	uint32_t end;
	uint32_t stride;
	uint32_t wmsk;
	uint32_t *buf;
};

void rockchip_alloc_region_mem(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_reg_rgn_save(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_reg_rgn_restore(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_reg_rgn_restore_reverse(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_regs_dump(uint32_t base,
			uint32_t start_offset,
			uint32_t end_offset,
			uint32_t stride);
void rockchip_dump_reg_rgns(struct reg_region *rgns, uint32_t rgn_num);

#endif /* PLAT_PM_HELPERS_H */
