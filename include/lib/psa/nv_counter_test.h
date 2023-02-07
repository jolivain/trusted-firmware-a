/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef NV_COUNTER_TEST_H
#define NV_COUNTER_TEST_H

/*
 * Test for AP/RSS interface for Non-Volatile counters
 * that reads, increments, and reads again each 3 types
 * of NV counters for: CCA, secure, and non-secure
 * firmware.
 */
int nv_counter_test(void);

#endif /* NV_COUNTER_TEST_H */
