/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_REPORT_H
#define ERRATA_REPORT_H

#ifndef __ASSEMBLER__

#if REPORT_ERRATA
void print_errata_status(void);
#else
static inline void print_errata_status(void) {}
#endif /* REPORT_ERRATA */

void errata_print_msg(unsigned int status, const char *cpu, const char *id);

/*
 * NOTE that this structure will be different on Aarch32 and Aarch64. The
 * uintptr_t will reflect the change and the alignment will be correct in both.
 */
struct erratum_entry {
	uintptr_t (*wa_func)(uint64_t cpu_rev);
	uintptr_t (*check_func)(uint64_t cpu_rev);
	/* Will fit CVEs with up to 10 character in the ID field */
	uint32_t erratum_id;
	/* we denote errata with 0, CVEs have their year here */
	uint16_t cve;
	uint8_t chosen;
	/* TODO(errata ABI): placeholder for the mitigated field */
	uint8_t _mitigated;
} __packed;

#endif /* __ASSEMBLER__ */

/* Errata status */
#define ERRATA_NOT_APPLIES	0
#define ERRATA_APPLIES		1
#define ERRATA_MISSING		2

/* Macro to get CPU revision code for checking errata version compatibility. */
#define CPU_REV(r, p)		((r << 4) | p)

#define ERRATUM(id)		0, id
#define CVE(year, id)		year, id

#endif /* ERRATA_REPORT_H */
