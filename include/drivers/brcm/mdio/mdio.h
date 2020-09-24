/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MDIO_H
#define MDIO_H

#define CMIC_MIIM_PARAM		(PLAT_CMIC_MIIM_BASE + 0x23c)
#define MDIO_PARAM_MIIM_CYCLE	29
#define MDIO_PARAM_INTERNAL_SEL	25
#define MDIO_PARAM_BUSID	22
#define MDIO_PARAM_BUSID_MASK	0x7
#define MDIO_PARAM_C45_SEL	21
#define MDIO_PARAM_PHYID	16
#define MDIO_PARAM_PHYID_MASK	0x1F
#define MDIO_PARAM_DATA		0
#define MDIO_PARAM_DATA_MASK	0xFFFF
#define CMIC_MIIM_READ_DATA	(PLAT_CMIC_MIIM_BASE + 0x240)
#define MDIO_READ_DATA_MASK	0xffff
#define CMIC_MIIM_ADDRESS	(PLAT_CMIC_MIIM_BASE + 0x244)
#define CMIC_MIIM_CTRL		(PLAT_CMIC_MIIM_BASE + 0x248)
#define MDIO_CTRL_WRITE_OP	0x1
#define MDIO_CTRL_READ_OP	0x2
#define CMIC_MIIM_STAT		(PLAT_CMIC_MIIM_BASE + 0x24c)
#define MDIO_STAT_DONE		1

int mdio_write(uint16_t busid, uint16_t phyid, uint32_t reg, uint16_t val);
int mdio_read(uint16_t busid, uint16_t phyid, uint32_t reg);
#endif /* MDIO_H */
