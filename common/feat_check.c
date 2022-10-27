/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************************************************************
 * The ARM Architecture Reference Manual mentions that CPU feature ID register
 * fields are "monotonic", as a higher value indicates support for all features
 * of previous versions.
 * However a higher version might introduce new (system) registers, which we
 * might need to context switch between secure and non-secure world. So we
 * cannot easily claim support for every higher field value, as non-secure
 * world can do.
 * To mitigate the issue, we run those checks here to find higher field values
 * than we know about and support today. The effect of the new value should
 * then be evaluated to see if we need extra code to support it. If this code
 * is merged, or if no new code is needed, the value in there should then be
 * upgraded.
 *****************************************************************************/

#include <arch_features.h>
#include <common/debug.h>
#include <common/feat_detect.h>

enum id_reg {
	ID_AA64PFR0,
	ID_AA64PFR1,
	ID_AA64PFR2,			/* RES0 as of ARM ARM issue I.a */
	ID_AA64DFR0,
	ID_AA64DFR1,			/* RES0 as of ARM ARM issue I.a */
	ID_AA64AFR0,
	ID_AA64AFR1,			/* RES0 as of ARM ARM issue I.a */
	ID_AA64ISAR0,
	ID_AA64ISAR1,
	ID_AA64ISAR2,
	ID_AA64ISAR3,			/* next unused, RES0 */
	ID_AA64MMFR0,
	ID_AA64MMFR1,
	ID_AA64MMFR2,
	ID_AA64MMFR3,			/* next unused, RES0 */
	NR_IDREGS
};

struct idreg_limit
{
	const char *idreg_name;
	uint64_t max_value;
} idreg_limits[] = {
	//			            |   |   |   |   |
	[ID_AA64PFR0]  = { "ID_AA64PFR0",  0x1311211123112222UL },
	[ID_AA64PFR1]  = { "ID_AA64PFR1",  0x0000001211011321UL },
	[ID_AA64PFR2]  = { "ID_AA64PFR2",  0x0000000000000000UL },
	//			            |   |   |   |   |
	[ID_AA64DFR0]  = { "ID_AA64DFR0",  0x10211104f0f0f81aUL },
	[ID_AA64DFR1]  = { "ID_AA64DFR1",  0x0000000000000000UL },
	//			            |   |   |   |   |
	[ID_AA64AFR0]  = { "ID_AA64AFR0",  0x00000000ffffffffUL },
	[ID_AA64AFR1]  = { "ID_AA64AFR1",  0x0000000000000000UL },
	//			            |   |   |   |   |
	[ID_AA64ISAR0] = { "ID_AA64ISAR0", 0x1221111111212120UL },
	[ID_AA64ISAR1] = { "ID_AA64ISAR1", 0x3111211111211552UL },
	[ID_AA64ISAR2] = { "ID_AA64ISAR2", 0x0000000001115112UL },
	[ID_AA64ISAR3] = { "ID_AA64ISAR3", 0x0000000000000000UL },
	//			            |   |   |   |   |
	[ID_AA64MMFR0] = { "ID_AA64MMFR0", 0x2100132310211126UL },
	[ID_AA64MMFR1] = { "ID_AA64MMFR1", 0x0111111111312122UL },
	[ID_AA64MMFR2] = { "ID_AA64MMFR2", 0x1221011112111111UL },
	[ID_AA64MMFR3] = { "ID_AA64MMFR3", 0x0000000000000000UL },
	//			            |   |   |   |   |
	//			           64  48  32  16   0
};

void check_arch_features(void)
{
	unsigned int i, bit;
	uint64_t reg;
	bool tainted = false;

	for (i = 0U; i < NR_IDREGS; i++) {
		struct idreg_limit *id_reg = &idreg_limits[i];

		switch(i) {
		case ID_AA64PFR0: reg = read_id_aa64pfr0_el1(); break;
		case ID_AA64PFR1: reg = read_id_aa64pfr1_el1(); break;
		case ID_AA64PFR2: reg = read_id_aa64pfr2_el1(); break;
		case ID_AA64DFR0: reg = read_id_aa64dfr0_el1(); break;
		case ID_AA64DFR1: reg = read_id_aa64dfr1_el1(); break;
		case ID_AA64AFR0: reg = read_id_aa64afr0_el1(); break;
		case ID_AA64AFR1: reg = read_id_aa64afr1_el1(); break;
		case ID_AA64ISAR0: reg = read_id_aa64isar0_el1(); break;
		case ID_AA64ISAR1: reg = read_id_aa64isar1_el1(); break;
		case ID_AA64ISAR2: reg = read_id_aa64isar2_el1(); break;
		case ID_AA64ISAR3: reg = read_id_aa64isar3_el1(); break;
		case ID_AA64MMFR0: reg = read_id_aa64mmfr0_el1(); break;
		case ID_AA64MMFR1: reg = read_id_aa64mmfr1_el1(); break;
		case ID_AA64MMFR2: reg = read_id_aa64mmfr2_el1(); break;
		case ID_AA64MMFR3: reg = read_id_aa64mmfr3_el1(); break;
		default:
			panic();
		}
		for (bit = 0U; bit < 64U; bit += 4U) {
			uint64_t mask = 0xfUL << bit;

			/* Some features encode -1 (0xf) as "not present" */
			if ((reg & mask) == mask) {
				continue;
			}

			if ((reg & mask) <= (id_reg->max_value & mask)) {
				continue;
			}

			ERROR("unsupported feature variant: %s[%d:%d]=%ld > %ld\n",
			      id_reg->idreg_name, bit + 3U, bit,
			      (reg >> bit) & 0xfU,
			      (id_reg->max_value >> bit) & 0xfU);

			tainted = true;
		}
	}

	if (tainted) {
		panic();
	}

	INFO("BL31: verified %d feature registers\n", i);
}
