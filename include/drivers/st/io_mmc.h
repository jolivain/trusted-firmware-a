/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_MMC_H
#define IO_MMC_H

#include <drivers/io/io_driver.h>

typedef struct io_mmc_dev_spec {
	bool use_boot_part;
} io_mmc_dev_spec_t;

int register_io_dev_mmc(const io_dev_connector_t **dev_con);

#endif /* IO_MMC_H */
