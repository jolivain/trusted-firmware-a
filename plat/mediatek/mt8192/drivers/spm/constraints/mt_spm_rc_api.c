
#include <mt_spm_rc_internal.h>
#include <mt_spm_rc_api.h>
#include <mt_lpm_smc.h>
#include <mt_spm.h>

#define SPM_RC_VALID_SET(dest, src) ({ dest |= src; })
#define SPM_RC_VALID_CLR(dest, src) ({ dest &= ~src; })

int spm_rc_condition_modifier(int id, unsigned int act,
			      const void *val, int dest_rc_id,
			      struct mt_spm_cond_tables * const tlb)
{
	unsigned int rc_id, cond_id, cond = 0;
	unsigned int id_ver = (unsigned int)id;
	int res = 0;

	plat_spm_lock();
	rc_id = SPM_RC_UPDATE_COND_RC_ID_GET(id_ver);
	cond_id = SPM_RC_UPDATE_COND_ID_GET(id_ver);

	if ((dest_rc_id != rc_id) || !val || !tlb) {
		res = -1;
		goto unlock;
	}

	cond = *((unsigned int *)val);

	if (cond_id < PLAT_SPM_COND_MAX) {
		if (act & MT_LPM_SMC_ACT_SET) {
			tlb->table_cg[cond_id] |= cond;
		} else if (act & MT_LPM_SMC_ACT_CLR) {
			tlb->table_cg[cond_id] &= ~cond;
		} else {
			res = -1;
		}
	} else if ((cond_id - PLAT_SPM_COND_MAX) < PLAT_SPM_COND_PLL_MAX) {
		unsigned int pll_idx = cond_id - PLAT_SPM_COND_MAX;

		cond = !!cond;
		if (act & MT_LPM_SMC_ACT_SET) {
			tlb->table_pll |= (cond << pll_idx);
		} else if (act & MT_LPM_SMC_ACT_CLR) {
			tlb->table_pll &= ~(cond << pll_idx);
		} else {
			res = -1;
		}
	} else {
		res = -1;
	}

unlock:
	plat_spm_unlock();

	return res;
}

int spm_rc_constraint_status_get(int id, int type,
				 unsigned int act, int dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest || !src) {
		return -1;
	}

	spm_lock_get();

	switch (type) {
	case CONSTRAINT_GET_ENTER_CNT:
		if (id == MT_RM_CONSTRAINT_ID_ALL) {
			dest->enter_cnt += src->enter_cnt;
		} else {
			dest->enter_cnt = src->enter_cnt;
		}
		break;
	case CONSTRAINT_GET_VALID:
		dest->is_valid = src->is_valid;
		break;
	case CONSTRAINT_COND_BLOCK:
		dest->is_cond_block = src->is_cond_block;
		break;
	case CONSTRAINT_GET_COND_BLOCK_DETAIL:
		dest->cond_res = src->cond_res;
		break;
	case CONSTRAINT_GET_RESIDNECY:
		dest->residency = src->residency;
		if (act & MT_LPM_SMC_ACT_CLR) {
			src->residency = 0;
		}
		break;
	default:
		break;
	}

	spm_lock_release();
	return 0;
}

int spm_rc_constraint_status_set(int id, int type, unsigned int act,
				 int dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) && (id != dest_rc_id)) || !dest) {
		return -1;
	}

	spm_lock_get();

	switch (type) {
	case CONSTRAINT_UPDATE_VALID:
		if (src) {
			if (act & MT_LPM_SMC_ACT_SET) {
				SPM_RC_VALID_SET(dest->is_valid, src->is_valid);
			} else if (act & MT_LPM_SMC_ACT_CLR) {
				SPM_RC_VALID_CLR(dest->is_valid, src->is_valid);
			}
		}
		break;
	case CONSTRAINT_RESIDNECY:
		if (act & MT_LPM_SMC_ACT_CLR) {
			dest->residency = 0;
		}
		break;
	default:
		break;
	}

	spm_lock_release();

	return 0;
}

int spm_rc_constraint_valid_set(int id, int dest_rc_id, unsigned int valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) && (id != dest_rc_id)) || !dest) {
		return -1;
	}

	spm_lock_get();
	SPM_RC_VALID_SET(dest->is_valid, valid);
	spm_lock_release();

	return 0;
}
int spm_rc_constraint_valid_clr(int id, int dest_rc_id, unsigned int valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) && (id != dest_rc_id)) || !dest) {
		return -1;
	}

	spm_lock_get();
	SPM_RC_VALID_CLR(dest->is_valid, valid);
	spm_lock_release();

	return 0;
}

