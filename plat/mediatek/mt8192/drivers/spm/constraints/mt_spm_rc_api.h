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
#include <mt_spm_trace.h>
#include <mt_spm_internal.h>
#include <mt_timer.h>

enum MT_SPM_RC_FP_TYPE {
	MT_SPM_RC_FP_INIT = 0,
	MT_SPM_RC_FP_ENTER_START,
	MT_SPM_RC_FP_ENTER_NOTIFY,
	MT_SPM_RC_FP_ENTER_WAKE_SPM_BEFORE,
	MT_SPM_RC_FP_ENTER_WAKE_SPM_AFTER,
	MT_SPM_RC_FP_RESUME_START,
	MT_SPM_RC_FP_RESUME_NOTIFY,
	MT_SPM_RC_FP_RESUME_RESET_SPM_BEFORE,
	MT_SPM_RC_FP_RESUME_BACKUP_EDGE_INT,
};

#define MT_SPM_RC_INFO(_cpu, _stateid, _rc_id) ({\
	MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_INFO,\
		(_cpu<<28) | ((_stateid&0xfff)<<16) | (_rc_id&0xffff)); })

#define MT_SPM_RC_LAST_TIME(_time) ({\
	MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_LAST_TIME_H,\
				(unsigned int)(_time>>32));\
	MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_LAST_TIME_L,\
				(unsigned int)(_time&0xffffffff)); })

#define MT_SPM_RC_TAG(_cpu, _stateid, _rc_id) ({\
	uint64_t ktime = sched_clock();\
	MT_SPM_RC_INFO(_cpu, _stateid, _rc_id);\
	(void)ktime;\
	MT_SPM_RC_LAST_TIME(ktime); })

#define MT_SPM_RC_FP(fp) ({\
	MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_FP, fp); })

#define MT_SPM_RC_TAG_VALID(_valid) ({\
	MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_VALID, _valid); })

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
