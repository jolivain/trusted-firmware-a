#ifndef __MT_SPM_TRACE_H__
#define __MT_SPM_TRACE_H__
#include <lib/mmio.h>

#include <mt_spm_sysram.h>

#define MT_SPM_TRACE_MAGIC	0x10005731

/* spm trace common type */
enum MT_SPM_TRACE_COMMON_TYPE {
	MT_SPM_TRACE_COMM_HAED,
	MT_SPM_TRACE_COMM_FP,
	MT_SPM_TRACE_COMM_RC_LAST_TIME_H,
	MT_SPM_TRACE_COMM_RC_LAST_TIME_L,
	MT_SPM_TRACE_COMM_RC_INFO,
	MT_SPM_TRACE_COMM_RC_FP,
	MT_SPM_TRACE_COMM_RC_VALID,
};


/* spm trace suspend type */
enum MT_SPM_TRACE_SUSPEND_TYPE {
	MT_SPM_TRACE_SUSPEND_WAKE_SRC,
};


/* spm trace low power type */
enum MT_SPM_TRACE_LP_TYPE {
	MT_SPM_TRACE_LP_RINGBUF_IDX,
	MT_SPM_TRACE_LP_RINGBUF_0,
	MT_SPM_TRACE_LP_RINGBUF_1,
	MT_SPM_TRACE_LP_RINGBUF_2,
	MT_SPM_TRACE_LP_RINGBUF_3,
	MT_SPM_TRACE_LP_RINGBUF_4,
};

#define MT_SPM_TRACE_LP_RINGBUF_MAX	(5)

#ifndef ATF_PLAT_SPM_TRACE_UNSUPPORT
/* spm trace common */
#define MT_SPM_TRACE_INIT() ({\
	mt_spm_sysram_init();\
	MT_SPM_SYSRAM_COMM_W_U32(MT_SPM_TRACE_COMM_HAED,\
				MT_SPM_TRACE_MAGIC); })

#define MT_SPM_TRACE_COMMON_U32_WR(type, val)\
	MT_SPM_SYSRAM_COMM_W_U32(type, val)

#define MT_SPM_TRACE_COMMON_WR(_type, val, _sz)\
	MT_SPM_SYSRAM_COMM_W(_type, val, _sz)

#define MT_SPM_TRACE_COMMON_U32_RD(type, val)\
	MT_SPM_SYSRAM_COMM_R_U32(type, val)

#define MT_SPM_TRACE_COMMON_RD(_type, val)\
	MT_SPM_SYSRAM_COMM_R(_type, val)



/* spm trace suspend */
#define MT_SPM_TRACE_SUSPEND_U32_WR(type, val)\
	MT_SPM_SYSRAM_SUSPEND_W_U32(type, val)

#define MT_SPM_TRACE_SUSPEND_WR(_type, val, _sz)\
	MT_SPM_SYSRAM_SUSPEND_W(_type, val, _sz)

#define MT_SPM_TRACE_SUSPEND_U32_RD(type, val)\
	MT_SPM_SYSRAM_SUSPEND_R_U32(type, val)

#define MT_SPM_TRACE_SUSPEND_RD(_type, val)\
	MT_SPM_SYSRAM_SUSPEND_R(_type, val)



/* spm trace low power */
#define MT_SPM_TRACE_LP_U32_WR(type, val)\
	MT_SPM_SYSRAM_LP_W_U32(type, val)

#define MT_SPM_TRACE_LP_WR(_type, val, _sz)\
	MT_SPM_SYSRAM_LP_W(_type, val, _sz)

#define MT_SPM_TRACE_LP_U32_RD(type, val)\
	MT_SPM_SYSRAM_LP_R_U32(type, val)

#define MT_SPM_TRACE_LP_RD(_type, val)\
	MT_SPM_SYSRAM_LP_R(_type, val)

#else
/* spm trace common */
#define MT_SPM_TRACE_INIT()
#define MT_SPM_TRACE_COMMON_U32_WR(type, val)
#define MT_SPM_TRACE_COMMON_WR(val)
#define MT_SPM_TRACE_COMMON_U32_RD(type, val)
#define MT_SPM_TRACE_COMMON_RD(val)

/* spm trace suspend */
#define MT_SPM_TRACE_SUSPEND_U32_WR(type, val)
#define MT_SPM_TRACE_SUSPEND_WR(val)
#define MT_SPM_TRACE_SUSPEND_U32_RD(type, val)
#define MT_SPM_TRACE_SUSPEND_RD(val)

/* spm trace low power */
#define MT_SPM_TRACE_LP_U32_WR(type, val)
#define MT_SPM_TRACE_LP_WR(val)
#define MT_SPM_TRACE_LP_U32_RD(type, val)
#define MT_SPM_TRACE_LP_RD(val)

#endif

#endif

