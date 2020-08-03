#ifndef __MT_SPM_CONSERVATION_H__
#define __MT_SPM_CONSERVATION_H__

#include <mt_spm_internal.h>

int spm_conservation(int state_id, unsigned int ext_opand
	, struct spm_lp_scen *spm_lp, unsigned int resource_req);

void spm_conservation_finish(int state_id, unsigned int ext_opand,
				   struct spm_lp_scen *spm_lp,
				   struct wake_status **status);


int spm_conservation_get_result(struct wake_status **res);

int spm_conservation_fw_init(struct pcm_desc *pcmdesc);

void spm_conservation_pwrctrl_init(struct pwr_ctrl *pwrctrl);

#endif /* __MT_SPM_CONSERVATION_H__ */
