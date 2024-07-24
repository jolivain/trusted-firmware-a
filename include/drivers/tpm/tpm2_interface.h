/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TPM2_INTERFACE_H
#define TPM2_INTERFACE_H

#include "tpm2_chip.h"

typedef struct interface_ops {
    int (*get_info)             (chip_data_t *chip_data, uint8_t locality);
    int (*send)                 (chip_data_t *chip_data, const uint8_t *buf);
    int (*receive)              (chip_data_t *chip_data, uint8_t *buf, size_t count);
    int (*request_access)       (chip_data_t *chip_data, uint8_t locality);
    void (*release_locality)    (chip_data_t *chip_data, uint8_t locality);
} interface_ops_t;

struct interface_ops *tpm_interface_getops(chip_data_t *chip_data, uint8_t locality);

uint32_t tpm2_fifo_io(uintptr_t addr, uint8_t rw, uint8_t len, uint8_t val);

uint8_t tpm2_get_reg_data(uintptr_t addr, uint8_t *status);

#endif /* TPM2_INTERFACE_H */
