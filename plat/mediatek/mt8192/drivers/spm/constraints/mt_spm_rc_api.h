/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_SPM_RC_API_H__
#define __MT_SPM_RC_API_H__

#include <arch_helpers.h>
#include <mt_spm_cond.h>
#include <mt_spm_constraint.h>
#include <mt_spm_internal.h>
#include <mt_timer.h>

int spm_rc_condition_modifier(int id, unsigned int act,
			      const void *val, int dest_rc_id,
			      struct mt_spm_cond_tables * const tlb);

int spm_rc_constraint_status_get(int id, int type,
				 unsigned int act, int dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest);

int spm_rc_constraint_status_set(int id, int type,
				 unsigned int act, int dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest);

int spm_rc_constraint_valid_set(int id, int dest_rc_id,
				unsigned int valid,
				struct constraint_status * const dest);

int spm_rc_constraint_valid_clr(int id, int dest_rc_id,
				unsigned int valid,
				struct constraint_status * const dest);
#endif /* __MT_SPM_RC_API_H__ */
