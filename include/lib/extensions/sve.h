/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SVE_H
#define SVE_H

#include <context.h>

#if (ENABLE_SME_FOR_NS || ENABLE_SVE_FOR_NS)

#if CTX_INCLUDE_SIMD_REGS

/* Please don't change order of fields in this struct as that may violate
 * alignment requirements and affect how assembly code accesses members of this
 * struct.
 */
typedef struct {
	uint8_t vectors[32][SIMD_VECTOR_LEN_BYTES];
	uint8_t fpsr[8];
	uint8_t fpcr[8];
#if CTX_INCLUDE_FPREGS && CTX_INCLUDE_AARCH32_REGS
	/* 16 bytes to align to next 16 byte boundary when CTX_INCLUDE_SVE_REGS is 0 */
	uint8_t fpexc32_el2[16];
#endif
#if CTX_INCLUDE_SVE_REGS
	/* FFR and each of predicates is one-eigth of the SVE vector length */
	uint8_t predicates[16][SIMD_VECTOR_LEN_BYTES / 8];
	uint8_t ffr[SIMD_VECTOR_LEN_BYTES / 8];
	/* adding reserved bytes for alignment. since instances of this struct
	 * will be part of an array, it's possible that when indexing into that
	 * array, we start a struct with an address which is not aligned to
	 * 16 bytes.
	 */
	uint8_t reserved[7 * SIMD_VECTOR_LEN_BYTES / 8];
#endif /* CTX_INCLUDE_SVE_REGS */
} simd_regs_t __attribute__((aligned(16)));

#endif /* CTX_INCLUDE_SIMD_REGS */

void sve_init_el2_unused(void);
void sve_enable_per_world(per_world_context_t *per_world_ctx);
void sve_disable_per_world(per_world_context_t *per_world_ctx);
#else
static inline void sve_init_el2_unused(void)
{
}
static inline void sve_enable_per_world(per_world_context_t *per_world_ctx)
{
}
static inline void sve_disable_per_world(per_world_context_t *per_world_ctx)
{
}
#endif /* ( ENABLE_SME_FOR_NS | ENABLE_SVE_FOR_NS ) */

#if CTX_INCLUDE_SVE_REGS
void sve_context_save(simd_regs_t *regs);
void sve_context_restore(simd_regs_t *regs);
#endif

#endif /* SVE_H */
