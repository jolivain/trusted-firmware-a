#include <lib/mmio.h>
#include <mt_spm_notifier.h>
#include <mt_spm_sspm_intc.h>

#define MT_SPM_SSPM_MBOX_OFF(x)		(SSPM_MBOX_3_BASE + x)
#define MT_SPM_MBOX(slot)		MT_SPM_SSPM_MBOX_OFF((slot<<2UL))

#define SSPM_MBOX_SPM_LP_LOOKUP1	MT_SPM_MBOX(0)
#define SSPM_MBOX_SPM_LP_LOOKUP2	MT_SPM_MBOX(1)
#define SSPM_MBOX_SPM_LP1		MT_SPM_MBOX(2)
#define SSPM_MBOX_SPM_LP2		MT_SPM_MBOX(3)

#define MCUPM_MBOX_OFFSET_LP	0x0C55FDA4
#define MCUPM_MBOX_ENTER_LP	0x454e0000
#define MCUPM_MBOX_LEAVE_LP	0x4c450000
#define MCUPM_MBOX_SLEEP_MASK	0x0000FFFF

int mt_spm_sspm_notify(int type, const void *val)
{
	if (!val)
		return -1;

	switch (type) {
	case MT_SPM_NOTIFY_LP_ENTER:
	{
		unsigned lp_mode;

		if (!val)
			break;
		lp_mode = *((unsigned int *)val);
		mmio_write_32(SSPM_MBOX_SPM_LP1, lp_mode);
		mmio_write_32(MCUPM_MBOX_OFFSET_LP,
		    MCUPM_MBOX_ENTER_LP | (lp_mode & MCUPM_MBOX_SLEEP_MASK));
		DO_SPM_SSPM_LP_SUSPEND();
	}
		break;
	case MT_SPM_NOTIFY_LP_LEAVE:
	{
		unsigned lp_mode;

		if (!val)
			break;
		lp_mode = *((unsigned int *)val);
		mmio_write_32(SSPM_MBOX_SPM_LP1, lp_mode);
		mmio_write_32(MCUPM_MBOX_OFFSET_LP,
		    MCUPM_MBOX_LEAVE_LP | (lp_mode & MCUPM_MBOX_SLEEP_MASK));
		DO_SPM_SSPM_LP_RESUME();
	}
		break;
	default:
		break;
	}
	return 0;
}

