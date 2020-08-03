#include <assert.h>
#include <stdio.h>

#include <mt_spm_trace.h>
#include <lib/libc/string.h>
#include <mt_spm_sysram.h>


#define DECLARE_SPM_SYSRAM_SZ (MT_SPM_SYSRAM_SUSPEND_SZ\
				+ MT_SPM_SYSRAM_COMM_SZ\
				+ MT_SPM_SYSRAM_LP_SZ)

/* Check sysram sz at compile time */
CASSERT(DECLARE_SPM_SYSRAM_SZ <= MT_SPM_SYSRAM_SIZE,
		SPM_SYSRAM_SZ_DECLARE_TOO_LARGE);


#define MT_SPM_TRACE_LP_RINGBUF_SZ	(96)
#define MT_SPM_TRACE_LP_RINGBUF_START	(MT_SPM_SYSRAM_LP_BASE\
					+ MT_SPM_SYSRAM_SLOT(8))

#define MT_SPM_TRACE_LP_RINGBUF_0_SLOT \
	(MT_SPM_TRACE_LP_RINGBUF_START)

#define MT_SPM_TRACE_LP_RINGBUF_1_SLOT \
	(MT_SPM_TRACE_LP_RINGBUF_0_SLOT\
		+ MT_SPM_TRACE_LP_RINGBUF_SZ)

#define MT_SPM_TRACE_LP_RINGBUF_2_SLOT \
	(MT_SPM_TRACE_LP_RINGBUF_1_SLOT\
		+ MT_SPM_TRACE_LP_RINGBUF_SZ)

#define MT_SPM_TRACE_LP_RINGBUF_3_SLOT \
	(MT_SPM_TRACE_LP_RINGBUF_2_SLOT\
		+ MT_SPM_TRACE_LP_RINGBUF_SZ)

#define MT_SPM_TRACE_LP_RINGBUF_4_SLOT \
	(MT_SPM_TRACE_LP_RINGBUF_3_SLOT\
		+ MT_SPM_TRACE_LP_RINGBUF_SZ)

/* Check ring buffer sz at compile time */
CASSERT((MT_SPM_TRACE_LP_RINGBUF_4_SLOT
		+ MT_SPM_TRACE_LP_RINGBUF_SZ) <=
	     (MT_SPM_SYSRAM_BASE + MT_SPM_SYSRAM_SIZE),
		SPM_LP_RINGBUF_SZ_DECLARE_ERROR);


#define MT_SPM_TRACE_SUSPEND_START	(MT_SPM_SYSRAM_SUSPEND_BASE\
					+ MT_SPM_SYSRAM_SLOT(0))

int mt_spm_sysram_write(int section, int type,
			const void *val, unsigned int sz)
{
	if (section == MT_SPM_SYSRAM_LP) {
		uintptr_t ptr = (uintptr_t)NULL;
		unsigned int cpy_sz =
			(sz > MT_SPM_TRACE_LP_RINGBUF_SZ) ?
			MT_SPM_TRACE_LP_RINGBUF_SZ : sz;

		if (type == MT_SPM_TRACE_LP_RINGBUF_0)
			ptr = (uintptr_t)
				MT_SPM_TRACE_LP_RINGBUF_0_SLOT;
		else if (type == (uintptr_t)MT_SPM_TRACE_LP_RINGBUF_1)
			ptr = (uintptr_t)
				MT_SPM_TRACE_LP_RINGBUF_1_SLOT;
		else if (type == MT_SPM_TRACE_LP_RINGBUF_2)
			ptr = (uintptr_t)
				MT_SPM_TRACE_LP_RINGBUF_2_SLOT;
		else if (type == MT_SPM_TRACE_LP_RINGBUF_3)
			ptr = (uintptr_t)
				MT_SPM_TRACE_LP_RINGBUF_3_SLOT;
		else if (type == MT_SPM_TRACE_LP_RINGBUF_4)
			ptr = (uintptr_t)
				MT_SPM_TRACE_LP_RINGBUF_4_SLOT;

		if (ptr)
			memcpy((void *)ptr, (const void *)val, cpy_sz);
	} else if (section == MT_SPM_SYSRAM_SUSPEND) {
		unsigned int cpy_sz =
			(sz > MT_SPM_SYSRAM_SUSPEND_SZ) ?
				MT_SPM_SYSRAM_SUSPEND_SZ : sz;

		memcpy((void *)MT_SPM_TRACE_SUSPEND_START,
				(const void *)val, cpy_sz);
	}
	return 0;
}

int mt_spm_sysram_read(int section, int type, void *val)
{
	return 0;
}

int mt_spm_sysram_init(void)
{
	memset((void *)MT_SPM_SYSRAM_BASE,
			0, MT_SPM_SYSRAM_SIZE);
	return 0;
}

