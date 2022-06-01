/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <errno.h>
#include <stdint.h>
#include <lib/utils_def.h>

/* define GIC SMC sub-function ID */
#define DUMP_IRQ_STATUS		U(0)

#define INTERRUPT_LEVEL_TRIGGER    U(0)
#define INTERRUPT_EDGE_TRIGGER     U(1)
#define INTERRUPT_FLAG_TRIGGER_BITMASK U(0x1)
#define IS_INTR_LEVEL(x) ((x & INTERRUPT_FLAG_TRIGGER_BITMASK) == INTERRUPT_LEVEL_TRIGGER)
#define IS_INTR_EDGE(x) ((x & INTERRUPT_FLAG_TRIGGER_BITMASK) == INTERRUPT_EDGE_TRIGGER)

typedef int (*fiq_handler_t)(void *cookie);

struct intr_flag {
	uint32_t trigger_type:1;
};

struct interrupt_controller {
	int (*config_intr)(uint32_t intr_num, struct intr_flag flag);
	int (*enable_intr)(uint32_t intr_num);
	int (*disable_intr)(uint32_t intr_num);
	uint32_t (*ack_intr)(void);
	void (*eoi_intr)(uint32_t intr_num);
	int (*validate_intr)(uint32_t intr_num);
	uint64_t (*dump_intr)(uint32_t intr_num);
};

int _request_fiq(uint32_t intr_num, fiq_handler_t handler,
	struct intr_flag flag, void *cookie, const char *name);
int intr_dispatcher(uint32_t intr_num);
void setup_interrupt_controller(struct interrupt_controller *intr_contr_impl);

const struct interrupt_controller *intr_contr_singleton(void);
#define request_fiq(_intr_num, _fiq_handler, _flag, _cookie) \
	_request_fiq(_intr_num, _fiq_handler, _flag, _cookie, #_fiq_handler)

#define EINTR_ERR_BASE (ELAST + 1)
#define EINTR_HANDLER_NOT_FOUND (EINTR_ERR_BASE + 0)
#define EINTR_BAD_INTID (EINTR_ERR_BASE + 1)
#define EINTR_NOT_SETUP (EINTR_ERR_BASE + 2)

#endif /* INTERRUPT_H */
