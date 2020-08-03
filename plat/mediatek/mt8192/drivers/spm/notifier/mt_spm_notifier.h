#ifndef __MT_SPM_SSPM_NOTIFIER_H__
#define __MT_SPM_SSPM_NOTIFIER_H__

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
enum MT_SPM_SSPM_NOTIFY_ID {
	MT_SPM_NOTIFY_LP_ENTER,
	MT_SPM_NOTIFY_LP_LEAVE,
};

int mt_spm_sspm_notify(int type, const void *val);

static inline int mt_spm_sspm_notify_u32(int type, unsigned int val)
{
	unsigned r_val = val;

	return mt_spm_sspm_notify(type, (const void *)&r_val);
}
#endif /* ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT */
#endif /* __MT_SPM_SSPM_NOTIFIER_H__ */
