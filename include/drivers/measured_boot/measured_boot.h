/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEASURED_BOOT_H
#define MEASURED_BOOT_H

#include <stdint.h>

void event_log_init(void);
int event_log_finalise(uint8_t **log_addr, size_t *log_size);

void measured_boot_init(void);
void measured_boot_finish(void);

int record_measurement(uintptr_t image_base, uint32_t image_size,
			uint32_t image_id);

#endif /* MEASURED_BOOT_H */
