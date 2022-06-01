#
# Copyright (c) 2020, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := topology

# Add your source code here

ifneq (,$(wildcard $(LOCAL_DIR)/$(CPU_PWR_TOPOLOGY)/topology_conf.mk))
include $(LOCAL_DIR)/$(CPU_PWR_TOPOLOGY)/topology_conf.mk
endif

ifneq ($(CPU_PWR_TOPOLOGY),)
LOCAL_SRCS-y := $(LOCAL_DIR)/$(CPU_PWR_TOPOLOGY)/topology.c
else
LOCAL_SRCS-y := $(LOCAL_DIR)/$(ARCH_VERSION)/topology.c
endif

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
