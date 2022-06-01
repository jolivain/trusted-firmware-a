/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GZ_SECIO_H
#define GZ_SECIO_H

/* Function prototype */
uint64_t sip_secio_write(u_register_t x1, u_register_t x2, u_register_t x3,
			 u_register_t x4, void *handle);
uint64_t sip_secio_read(u_register_t x1, u_register_t x2, u_register_t x3,
			u_register_t x4, void *handle, uint32_t *rc_1);

#endif /* GZ_SECIO_H */
