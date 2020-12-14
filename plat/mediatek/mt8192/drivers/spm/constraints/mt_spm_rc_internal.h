/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_SPM_RC_INTERNAL_H__
#define __MT_SPM_RC_INTERNAL_H__

#define SPM_FLAG_SRAM_SLEEP_CTRL			\
	(SPM_FLAG_DISABLE_SSPM_SRAM_SLEEP |		\
	 SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP |	\
	 SPM_FLAG_DISABLE_SYSRAM_SLEEP |		\
	 SPM_FLAG_DISABLE_MCUPM_SRAM_SLEEP |		\
	 SPM_FLAG_DISABLE_SRAM_EVENT)

#define SPM_RC_UPDATE_COND_ID_MASK	0xffff
#define SPM_RC_UPDATE_COND_RC_ID_MASK	0xffff
#define SPM_RC_UPDATE_COND_RC_ID_SHIFT	16

#define SPM_RC_UPDATE_COND_RC_ID_GET(val)		\
	((val >> SPM_RC_UPDATE_COND_RC_ID_SHIFT) &	\
	 SPM_RC_UPDATE_COND_RC_ID_MASK)

#define SPM_RC_UPDATE_COND_ID_GET(val) (val & SPM_RC_UPDATE_COND_ID_MASK)

/* cpu buck/ldo constraint function */
int spm_is_valid_rc_cpu_buck_ldo(int cpu, int state_id);
int spm_update_rc_cpu_buck_ldo(int state_id, int type, const void *val);
unsigned int spm_allow_rc_cpu_buck_ldo(int state_id);
int spm_run_rc_cpu_buck_ldo(int cpu, int state_id);
int spm_reset_rc_cpu_buck_ldo(int cpu, int state_id);
int spm_get_status_rc_cpu_buck_ldo(int type, void *priv);

/* spm resource dram constraint function */
int spm_is_valid_rc_dram(int cpu, int state_id);
int spm_update_rc_dram(int state_id, int type, const void *val);
unsigned int spm_allow_rc_dram(int state_id);
int spm_run_rc_dram(int cpu, int state_id);
int spm_reset_rc_dram(int cpu, int state_id);
int spm_get_status_rc_dram(int type, void *priv);

/* spm resource syspll constraint function */
int spm_is_valid_rc_syspll(int cpu, int state_id);
int spm_update_rc_syspll(int state_id, int type, const void *val);
unsigned int spm_allow_rc_syspll(int state_id);
int spm_run_rc_syspll(int cpu, int state_id);
int spm_reset_rc_syspll(int cpu, int state_id);
int spm_get_status_rc_syspll(int type, void *priv);

/* spm resource bus26m constraint function */
int spm_is_valid_rc_bus26m(int cpu, int state_id);
int spm_update_rc_bus26m(int state_id, int type, const void *val);
unsigned int spm_allow_rc_bus26m(int state_id);
int spm_run_rc_bus26m(int cpu, int state_id);
int spm_reset_rc_bus26m(int cpu, int state_id);
int spm_get_status_rc_bus26m(int type, void *priv);
#endif /* __MT_SPM_RC_INTERNAL_H__ */
