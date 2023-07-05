/*
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SIMD_CTX_H

enum simd_ctx_op {
	SIMD_CTX_OP_SAVE,
	SIMD_CTX_OP_RESTORE,
};

void simd_ctx_switch(enum simd_ctx_op op, uint32_t security_state);

#endif
