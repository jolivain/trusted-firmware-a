#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mtk_init

LOCAL_SRCS-y := $(LOCAL_DIR)/mtk_init.c
LOCAL_SRCS-y += $(LOCAL_DIR)/mtk_mmap_init.c

# Extra linker script
ifneq ($(MTK_EXTRA_LINKERFILE),)
# Local linker script added
$(eval EXTRA_LINKERFILE += ${LOCAL_DIR}/mtk_mmap_pool.ld)

# Generate final linker script by C preprocessor compiler
$(eval MTK_LINKERFILE_SOURCE += ${LOCAL_DIR}/mtk_init.ld.S)
$(eval MTK_LINKERFILE_SOURCE += ${LOCAL_DIR}/mtk_ro.ld.S)
endif

# Epilogue, Build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
