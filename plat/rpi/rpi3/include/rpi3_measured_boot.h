/*
 * Copyright (c) 2022 - 2024, Linaro, Arm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI3_MEASURED_BOOT_H
#define RPI3_MEASURED_BOOT_H

#include <stdint.h>

#include <arch_helpers.h>

int rpi3_set_nt_fw_info(size_t log_size, uintptr_t *ns_log_addr);

#endif /* RPI3_MEASURED_BOOT_H */
