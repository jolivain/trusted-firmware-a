/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_INTERNAL_H
#define MT_SPM_INTERNAL_H

#define POWER_ON_VAL1_DEF	0x00815828
#define PCM_FSM_STA_DEF		0x00048490

#define SPM_WAKEUP_EVENT_MASK_DEF	0xC0000000

/* PCM_PWR_IO_EN */
#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

/* SPM_SWINT */
#define PCM_SW_INT0		(1U << 0)
#define PCM_SW_INT1		(1U << 1)
#define PCM_SW_INT2		(1U << 2)
#define PCM_SW_INT3		(1U << 3)
#define PCM_SW_INT4		(1U << 4)
#define PCM_SW_INT5		(1U << 5)
#define PCM_SW_INT6		(1U << 6)
#define PCM_SW_INT7		(1U << 7)
#define PCM_SW_INT8		(1U << 8)
#define PCM_SW_INT9		(1U << 9)
#define PCM_SW_INT_ALL		(PCM_SW_INT9 | PCM_SW_INT8 | PCM_SW_INT7 | \
				 PCM_SW_INT6 | PCM_SW_INT5 | PCM_SW_INT4 | \
				 PCM_SW_INT3 | PCM_SW_INT2 | PCM_SW_INT1 | \
				 PCM_SW_INT0)

/* SPM_IRQ_MASK */
#define ISRM_TWAM		(1U << 2)
#define ISRM_PCM_RETURN		(1U << 3)
#define ISRM_RET_IRQ0		(1U << 8)
#define ISRM_RET_IRQ1		(1U << 9)
#define ISRM_RET_IRQ2		(1U << 10)
#define ISRM_RET_IRQ3		(1U << 11)
#define ISRM_RET_IRQ4		(1U << 12)
#define ISRM_RET_IRQ5		(1U << 13)
#define ISRM_RET_IRQ6		(1U << 14)
#define ISRM_RET_IRQ7		(1U << 15)
#define ISRM_RET_IRQ8		(1U << 16)
#define ISRM_RET_IRQ9		(1U << 17)
#define ISRM_RET_IRQ_AUX	((ISRM_RET_IRQ9) | (ISRM_RET_IRQ8) | \
				 (ISRM_RET_IRQ7) | (ISRM_RET_IRQ6) | \
				 (ISRM_RET_IRQ5) | (ISRM_RET_IRQ4) | \
				 (ISRM_RET_IRQ3) | (ISRM_RET_IRQ2) | \
				 (ISRM_RET_IRQ1))
#define ISRM_ALL_EXC_TWAM	(ISRM_RET_IRQ_AUX /*| ISRM_RET_IRQ0 | ISRM_PCM_RETURN*/)
#define ISRM_ALL		(ISRM_ALL_EXC_TWAM | ISRM_TWAM)

/* SPM_IRQ_STA */
#define ISRS_TWAM		(1U << 2)
#define ISRS_PCM_RETURN		(1U << 3)
#define ISRS_SW_INT0		(1U << 4)
#define ISRC_TWAM		ISRS_TWAM
#define ISRC_ALL_EXC_TWAM	ISRS_PCM_RETURN
#define ISRC_ALL		(ISRC_ALL_EXC_TWAM | ISRC_TWAM)

struct pcm_desc {
	const char *version;	  /* PCM code version */
	const uint32_t *base;	  /* binary array base */
	const uint32_t base_dma;  /* dma addr of base */
	const uint16_t size;	  /* binary array size */
	const uint8_t sess;	  /* session number */
	const uint8_t replace;	  /* replace mode */
	const uint16_t addr_2nd;  /* 2nd binary array size */
	const uint16_t reserved;  /* for 32bit alignment */

	uint32_t vec0;		/* event vector 0 config */
	uint32_t vec1;		/* event vector 1 config */
	uint32_t vec2;		/* event vector 2 config */
	uint32_t vec3;		/* event vector 3 config */
	uint32_t vec4;		/* event vector 4 config */
	uint32_t vec5;		/* event vector 5 config */
	uint32_t vec6;		/* event vector 6 config */
	uint32_t vec7;		/* event vector 7 config */
	uint32_t vec8;		/* event vector 8 config */
	uint32_t vec9;		/* event vector 9 config */
	uint32_t vec10;		/* event vector 10 config */
	uint32_t vec11;		/* event vector 11 config */
	uint32_t vec12;		/* event vector 12 config */
	uint32_t vec13;		/* event vector 13 config */
	uint32_t vec14;		/* event vector 14 config */
	uint32_t vec15;		/* event vector 15 config */
};

void spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc);
void spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc);
void spm_init_pcm_register(void);
void spm_init_event_vector(const struct pcm_desc *pcmdesc);
void spm_kick_pcm_to_run(void);

#endif /* MT_SPM_INTERNAL_H */
