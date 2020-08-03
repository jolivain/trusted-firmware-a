#ifndef __MT_SPM_SYSRAM_H__
#define __MT_SPM_SYSRAM_H__

#include <platform_def.h>

#define MT_SPM_SYSRAM_SLOT(slot)	(slot<<2u)

enum MT_SPM_SYSRAM_TYPE {
	MT_SPM_SYSRAM_COMMON,
	MT_SPM_SYSRAM_SUSPEND,
	MT_SPM_SYSRAM_LP,
};

/* spm sram usage with mcdi sram
 * start offset : 0x500
 * size : 0x300
 */
#define MT_SPM_SYSRAM_BASE		(MTK_MCDI_SRAM_BASE + 0x500)
#define MT_SPM_SYSRAM_SIZE		(0x300)

/* spm common debug section
 * slot[0]: magic number
 * slot[1]: common footprint
 * slot[2]: time stamp for last constraint entry time (High bit)
 * slot[3]: time stamp for last constraint entry time (Low bit)
 * slot[4]: resource constraint information (cpuid | state id | constraint id)
 * slot[5]: resource constraint footprint
 * slot[6]: resource status.valid
 * slot[7]: reserve
 */
#define MT_SPM_SYSRAM_COMM_BASE		(MT_SPM_SYSRAM_BASE)
#define MT_SPM_SYSRAM_COMM_SZ		(0x20)

#define MT_SPM_SYSRAM_SUSPEND_BASE \
	(MT_SPM_SYSRAM_BASE + MT_SPM_SYSRAM_COMM_SZ)
#define MT_SPM_SYSRAM_SUSPEND_SZ	(0xe0)

#define MT_SPM_SYSRAM_LP_BASE \
	(MT_SPM_SYSRAM_SUSPEND_BASE + MT_SPM_SYSRAM_SUSPEND_SZ)
#define MT_SPM_SYSRAM_LP_SZ		(0x200)

#define MT_SPM_SYSRAM_W_U32(addr, val)	mmio_write_32(addr, val)
#define MT_SPM_SYSRAM_R_U32(addr, val)	({\
	unsigned int *r_val = (unsigned int *)val;\
	if (r_val)\
		*r_val = mmio_read_32(addr); })

/* common sysram section 0x500h */
#define MT_SPM_SYSRAM_COMM_W_U32(_type, _val) ({\
	MT_SPM_SYSRAM_W_U32((MT_SPM_SYSRAM_COMM_BASE +\
		MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_SYSRAM_COMM_W(_type, _val, _sz) ({\
	int ret = MT_SPM_SYSRAM_W(MT_SPM_SYSRAM_COMMON,\
			_type, _val, _sz); ret; })

#define MT_SPM_SYSRAM_COMM_R_U32(_type, _val) ({\
	MT_SPM_SYSRAM_R_U32((MT_SPM_SYSRAM_COMM_BASE +\
		MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_SYSRAM_COMM_R(_type, _val) ({\
	int ret = MT_SPM_SYSRAM_R(MT_SPM_SYSRAM_COMMON,\
			_type, _val); ret; })

/* suspend sysram section 0x520h */
#define MT_SPM_SYSRAM_SUSPEND_W_U32(_type, _val) ({\
	MT_SPM_SYSRAM_W_U32((MT_SPM_SYSRAM_SUSPEND_BASE +\
		MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_SYSRAM_SUSPEND_W(_type, _val, _sz) ({\
	int ret = MT_SPM_SYSRAM_W(MT_SPM_SYSRAM_SUSPEND,\
			_type, _val, _sz); ret; })

#define MT_SPM_SYSRAM_SUSPEND_R_U32(_type, _val) ({\
	MT_SPM_SYSRAM_R_U32((MT_SPM_SYSRAM_SUSPEND_BASE +\
		MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_SYSRAM_SUSPEND_R(_type, _val) ({\
	int ret = MT_SPM_SYSRAM_R(MT_SPM_SYSRAM_SUSPEND,\
			_type, _val); ret; })

/* low power sysram section 0x600h */
#define MT_SPM_SYSRAM_LP_W_U32(_type, _val) ({\
	MT_SPM_SYSRAM_W_U32((MT_SPM_SYSRAM_LP_BASE +\
		MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_SYSRAM_LP_W(_type, _val, _sz) ({\
	int ret = MT_SPM_SYSRAM_W(MT_SPM_SYSRAM_LP,\
			_type, _val, _sz); ret; })


#define MT_SPM_SYSRAM_LP_R_U32(_type, _val) ({\
	MT_SPM_SYSRAM_R_U32((MT_SPM_SYSRAM_LP_BASE +\
		MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_SYSRAM_LP_R(_type, _val) ({\
	int ret = MT_SPM_SYSRAM_R(MT_SPM_SYSRAM_LP,\
			_type, _val); ret; })

#endif /* __MT_SPM_SYSRAM_H__ */
