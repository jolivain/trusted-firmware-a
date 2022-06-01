#
# Copyright (c) 2021, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := interrupt

# Add your source code here
LOCAL_SRCS-y := $(LOCAL_DIR)/interrupt.c
$(eval $(call add_defined_option,CONFIG_MTK_INTERRUPT))

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
# Include sub rules.mk
SUB_RULES-y := $(LOCAL_DIR)/impl
# Expand sub rules.mk
$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
