/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_COMMON_H
#define MT_SPM_RC_COMMON_H

#include <stdbool.h>
/* cpu buck/ldo constraint function */
bool spm_is_valid_rc_cpu_buck_ldo(unsigned int cpu, int state_id);
unsigned int spm_allow_rc_cpu_buck_ldo(int state_id);
int spm_run_rc_cpu_buck_ldo(unsigned int cpu, int state_id);
int spm_reset_rc_cpu_buck_ldo(unsigned int cpu, int state_id);

/* spm resource dram constraint function */
bool spm_is_valid_rc_dram(unsigned int cpu, int state_id);
int spm_update_rc_dram(int state_id, int type, const void *val);
unsigned int spm_allow_rc_dram(int state_id);
int spm_run_rc_dram(unsigned int cpu, int state_id);
int spm_reset_rc_dram(unsigned int cpu, int state_id);

/* spm resource syspll constraint function */
bool spm_is_valid_rc_syspll(unsigned int cpu, int state_id);
int spm_update_rc_syspll(int state_id, int type, const void *val);
unsigned int spm_allow_rc_syspll(int state_id);
int spm_run_rc_syspll(unsigned int cpu, int state_id);
int spm_reset_rc_syspll(unsigned int cpu, int state_id);

/* spm resource bus26m constraint function */
bool spm_is_valid_rc_bus26m(unsigned int cpu, int state_id);
int spm_update_rc_bus26m(int state_id, int type, const void *val);
unsigned int spm_allow_rc_bus26m(int state_id);
int spm_run_rc_bus26m(unsigned int cpu, int state_id);
int spm_reset_rc_bus26m(unsigned int cpu, int state_id);

#endif /* MT_SPM_RC_COMMON_H */
