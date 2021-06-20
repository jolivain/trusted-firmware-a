/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWU_PRIVATE_H
#define FWU_PRIVATE_H

extern bool is_fwu_initialized;
extern struct fwu_metadata metadata;

int fwu_metadata_load(unsigned int image_id);

#endif /* FWU_PRIVATE_H */
