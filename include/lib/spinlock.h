/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPINLOCK_H
#define SPINLOCK_H

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct spinlock {
	volatile uint32_t lock;
} spinlock_t;

#ifdef __COVERITY__
void spin_lock(spinlock_t *lock)
{
	__coverity_exclusive_lock_acquire__(lock);
}

void spin_unlock(spinlock_t *lock)
{
	__coverity_exclusive_lock_release__(lock);
}
#else
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
#endif /* __COVERITY__ */

#else

/* Spin lock definitions for use in assembly */
#define SPINLOCK_ASM_ALIGN	2
#define SPINLOCK_ASM_SIZE	4

#endif

#endif /* SPINLOCK_H */
