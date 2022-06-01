/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
/* MTK header files */
#include <drivers/interrupt.h>
#include <lib/mtk_init/mtk_init.h>

#define MTK_INTERRUPT_POOL_MAX 5

struct mtk_interrupt {
	uint32_t intr_num;
	struct intr_flag flag;
	const char *handler_name;
	fiq_handler_t handler;
	void *cookie;
};

static struct mtk_interrupt interrupt_pool[MTK_INTERRUPT_POOL_MAX];
static struct interrupt_controller *intr_contr_p;
static uint8_t interrupt_pool_last_index;

/* Validate mandatory callbacks */
static inline int validate_interrupt_controller(struct interrupt_controller *intr_contr)
{
	if (intr_contr == NULL) {
		ERROR("intr_contr is not implemented\n");
		return -EINTR_NOT_SETUP;
	}
	if (intr_contr->config_intr == NULL) {
		ERROR("config_intr is not implemented\n");
		return -EINVAL;
	}
	if (intr_contr->ack_intr == NULL) {
		ERROR("ack_intr is not implemented\n");
		return -EINVAL;
	}
	if (intr_contr->enable_intr == NULL) {
		ERROR("enable_intr is not implemented\n");
		return -EINVAL;
	}
	if (intr_contr->eoi_intr == NULL) {
		ERROR("eoi_intr is not implemented\n");
		return -EINVAL;
	}
	return 0;
}

int _request_fiq(uint32_t intr_num, fiq_handler_t handler,
	struct intr_flag flag, void *cookie, const char *name)
{
	int err_code = 0;

	if (!intr_contr_p) {
		ERROR("intr_contr is not implemented\n");
		err_code = -EINTR_NOT_SETUP;
		goto _request_fiq_end;
	}
	/* Only register SPI and SGI, PPI  */
	if (intr_contr_p->validate_intr) {
		err_code = intr_contr_p->validate_intr(intr_num);
		if (err_code)
			goto _request_fiq_end;
	}
	if (handler == NULL) {
		ERROR("Interrupt handler cannot be NULL\n");
		assert(handler != NULL);
		err_code = -EINVAL;
		goto _request_fiq_end;
	}
	if (name == NULL) {
		NOTICE("Interrupt handler name should not be NULL\n");
	}
	if (interrupt_pool_last_index >= MTK_INTERRUPT_POOL_MAX) {
		ERROR("Exceed interrupt_pool maxi size[%u], max size is %u\n",
			interrupt_pool_last_index, MTK_INTERRUPT_POOL_MAX);
		assert(interrupt_pool_last_index < MTK_INTERRUPT_POOL_MAX);
		err_code = -ENOMEM;
		goto _request_fiq_end;
	}
	interrupt_pool[interrupt_pool_last_index].intr_num = intr_num;
	interrupt_pool[interrupt_pool_last_index].handler = handler;
	interrupt_pool[interrupt_pool_last_index].flag = flag;
	interrupt_pool[interrupt_pool_last_index].handler_name = name;
	interrupt_pool[interrupt_pool_last_index].cookie = cookie;
	interrupt_pool_last_index++;
	/* Configure interrupt */
	if (intr_contr_p->config_intr) {
		err_code = intr_contr_p->config_intr(intr_num, flag);
		if (err_code)
			goto _request_fiq_end;
	}
_request_fiq_end:
	return err_code;
}

void setup_interrupt_controller(struct interrupt_controller *intc_impl)
{
	assert(intc_impl != NULL);
	intr_contr_p = intc_impl;

	if (validate_interrupt_controller(intr_contr_p)) {
		ERROR("Interrupt controller's mandatory functions are not implemented\n");
		assert(validate_interrupt_controller(intr_contr_p) == 0);
	}
}

const struct interrupt_controller *intr_contr_singleton(void)
{
	return intr_contr_p;
}

int intr_dispatcher(uint32_t intr_num)
{
	unsigned int i;
	struct mtk_interrupt *intr_entry;
	int ret = -EINTR_HANDLER_NOT_FOUND;

	for (i = 0 ; i < interrupt_pool_last_index ; i++) {
		intr_entry = &interrupt_pool[i];
		if (intr_entry->intr_num == intr_num) {
			if (intr_entry->handler) {
				VERBOSE("intr num[%u] raised:%s\n",
					intr_num, intr_entry->handler_name);
				ret = intr_entry->handler(interrupt_pool[i].cookie);
				if (ret)
					ERROR("intr num[%u], %s was failed, error:%d\n",
						intr_num, intr_entry->handler_name, ret);
			} else
				ERROR("Interrupt num was found but handler is NULL\n");
			break;
		}
	}
	return ret;
}

static uint64_t intr_root_handler(uint32_t id,
			   uint32_t flags,
			   void *handle,
			   void *cookie)
{
	int err_code = 0;
	uint32_t intr_num = INTR_ID_UNAVAILABLE;

	if (!intr_contr_p) {
		err_code = -EINTR_NOT_SETUP;
		ERROR("intr_contr is not implemented\n");
		goto isr_handler_end;
	}
	if (intr_contr_p->ack_intr)
		intr_num = intr_contr_p->ack_intr();
	if (intr_contr_p->validate_intr) {
		err_code = intr_contr_p->validate_intr(intr_num);
		if (err_code)
			goto isr_handler_end;
	}
	err_code = intr_dispatcher(intr_num);
	if (err_code)
		goto isr_handler_err;
	if (intr_contr_p->eoi_intr)
		intr_contr_p->eoi_intr(intr_num);
	goto isr_handler_end;
isr_handler_err:
	ERROR("intr_dispatcher error:%d\n", err_code);
	if (intr_contr_p->dump_intr)
		intr_contr_p->dump_intr(intr_num);
isr_handler_end:
	return (uint64_t)err_code;
}

static int mtk_register_intr_type_handler(void)
{
	uint32_t flags;
	int	rc;

	/*
	 * Register an interrupt handler for EL3 interrupts
	 * when generated during code executing in the
	 * non-secure state.
	 */
	flags = 0;
	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3,
						intr_root_handler,
						flags);
	assert(rc == 0);
	return rc;
}
MTK_ARCH_INIT(mtk_register_intr_type_handler);

u_register_t handle_gic_operation(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *smccc_ret)
{
	u_register_t ret = 0;

	switch (x1) {
	case DUMP_IRQ_STATUS:
		if (intr_contr_p->dump_intr)
			ret = intr_contr_p->dump_intr((uint32_t)x2);
		break;
	}
	return ret;
}
DECLARE_SMC_HANDLER(MTK_SIP_KERNEL_GIC_OP, handle_gic_operation);
