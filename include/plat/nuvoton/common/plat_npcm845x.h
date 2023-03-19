/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (c) 2017-2023 Nuvoton Technology Corp.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_NPCM845X_H
#define PLAT_NPCM845X_H

#include <drivers/arm/gicv2.h>
#include <lib/psci/psci.h>

#ifdef NPCM845X_DEBUG
struct plat_gic_ctx {
	gicv2_redist_ctx_t rdist_ctx[PLATFORM_CORE_COUNT];
	gicv2_dist_ctx_t dist_ctx;
};
#endif /* NPCM845X_DEBUG */

unsigned int plat_calc_core_pos(uint64_t mpidr);
void npcm845x_mailbox_init(uintptr_t base_addr);
void plat_gic_driver_init(void);
void plat_gic_init(void);
void plat_gic_cpuif_enable(void);
void plat_gic_cpuif_disable(void);
void plat_gic_pcpu_init(void);

void __dead2 npcm845x_system_off(void);
void __dead2 npcm845x_system_reset(void);
void npcm845x_pwr_domain_on_finish(const psci_power_state_t *target_state);

#ifdef NPCM845X_DEBUG
void npcm845x_get_sys_suspend_power_state(psci_power_state_t *req_state);
void plat_gic_save(unsigned int proc_num, struct plat_gic_ctx *ctx);
void plat_gic_restore(unsigned int proc_num, struct plat_gic_ctx *ctx);
#endif /* NPCM845X_DEBUG */

bool npcm845x_is_wakeup_src_irqsteer(void);

void __dead2 npcm845x_pwr_down_wfi(const psci_power_state_t *target_state);
void npcm845x_cpu_standby(plat_local_state_t cpu_state);
int npcm845x_validate_ns_entrypoint(uintptr_t entrypoint);
int npcm845x_pwr_domain_on(u_register_t mpidr);

int npcm845x_validate_power_state(unsigned int power_state,
				  psci_power_state_t *req_state);

#if !ARM_BL31_IN_DRAM
void npcm845x_get_sys_suspend_power_state(psci_power_state_t *req_state);
#endif

void __dead2 npcm845x_pwr_domain_pwr_down_wfi(
	const psci_power_state_t *target_state);

void npcm845x_pwr_domain_suspend_finish(const psci_power_state_t *target_state);
void npcm845x_pwr_domain_suspend(const psci_power_state_t *target_state);
void npcm845x_pwr_domain_off(const psci_power_state_t *target_state);

void __init npcm845x_bl31_plat_arch_setup(void);

#endif /* PLAT_NPCM845X_H */
