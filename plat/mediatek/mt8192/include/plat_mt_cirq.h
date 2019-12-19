/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MT_CIRQ_H
#define PLAT_MT_CIRQ_H

#define SYS_CIRQ_BASE        0x10204000
#define CIRQ_IRQ_NUM         439
#define CIRQ_SPI_START       96
/*
 * Define hardware register
 */
#define  CIRQ_STA_BASE         0x000
#define  CIRQ_ACK_BASE         0x080
#define  CIRQ_MASK_BASE        0x100
#define  CIRQ_MASK_SET_BASE    0x180
#define  CIRQ_MASK_CLR_BASE    0x200
#define  CIRQ_SENS_BASE        0x280
#define  CIRQ_SENS_SET_BASE    0x300
#define  CIRQ_SENS_CLR_BASE    0x380
#define  CIRQ_POL_BASE         0x400
#define  CIRQ_POL_SET_BASE     0x480
#define  CIRQ_POL_CLR_BASE     0x500
#define  CIRQ_CON              0x600

/*
 * Register placement
 */
#define  CIRQ_CON_EN_BITS           0
#define  CIRQ_CON_EDGE_ONLY_BITS    1
#define  CIRQ_CON_FLUSH_BITS        2
#define  CIRQ_CON_EVENT_BITS        31
#define  CIRQ_CON_SW_RST_BITS       20
#define  CIRQ_CON_BITS_MASK         0x7

/*
 * Register setting
 */
#define  CIRQ_CON_EN            0x1
#define  CIRQ_CON_EDGE_ONLY     0x1
#define  CIRQ_SW_RESET          0x1
#define  CIRQ_CON_FLUSH         0x1

/*
 * Define constant
 */
#define  CIRQ_CTRL_REG_NUM      ((CIRQ_IRQ_NUM + 31) / 32)
#define  MT_CIRQ_POL_NEG                0
#define  MT_CIRQ_POL_POS                1
#define  MT_CIRQ_EDGE_SENSITIVE         0
#define  MT_CIRQ_LEVEL_SENSITIVE        1

/*
 * Define macro
 */
#define  IRQ_TO_CIRQ_NUM(irq)       ((irq) - (CIRQ_SPI_START))
#define  CIRQ_TO_IRQ_NUM(cirq)      ((cirq) + (CIRQ_SPI_START))

/*
 * Define cirq events
 */
struct cirq_events {
	uint32_t spi_start;
	uint32_t num_of_events;
	uint32_t *wakeup_events;
};

/*
 * Define function prototypes.
 */
void mt_cirq_enable(void);
void mt_cirq_disable(void);
void mt_cirq_clone_gic(void);
void mt_cirq_flush(void);
void mt_cirq_sw_reset(void);
void set_wakeup_sources(uint32_t *list, uint32_t num_of_events);
void mt_cirq_dump_reg(void);

#endif  /* PLAT_MT_CIRQ_H */
