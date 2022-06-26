#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := mtk_common

# Add your source code here
LOCAL_SRCS-y := ${LOCAL_DIR}/mtk_bl31_setup.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mtk_smc_handlers.c
LOCAL_SRCS-$(MTK_SIP_KERNEL_BOOT_ENABLE) += ${LOCAL_DIR}/cold_boot.c

ifneq ($(MTK_EXTRA_LINKERFILE),)
$(eval EXTRA_LINKERFILE += ${LOCAL_DIR}/mtk_smc_descriptor_pool.ld)
endif

# Epilogue, build sources, since needs IMAGE_AT_EL3
$(eval $(call MAKE_LOCALS,$(LOCAL_SRCS-y),$(MTK_BL)))
