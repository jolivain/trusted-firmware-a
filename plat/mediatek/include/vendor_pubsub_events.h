/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/el3_runtime/pubsub.h>

REGISTER_PUBSUB_EVENT(lpm_publish_event);
REGISTER_PUBSUB_EVENT(suspend_publish_event);
REGISTER_PUBSUB_EVENT(mt_cpupm_publish_pwr_on);
REGISTER_PUBSUB_EVENT(mt_cpupm_publish_pwr_off);
REGISTER_PUBSUB_EVENT(mt_cpupm_publish_afflv_pwr_on);
REGISTER_PUBSUB_EVENT(mt_cpupm_publish_afflv_pwr_off);
REGISTER_PUBSUB_EVENT(publish_check_wakeup_irq);
REGISTER_PUBSUB_EVENT(watchdog_timeout);
