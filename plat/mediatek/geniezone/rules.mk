#
# Copyright (c) 2021, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

#Define your module name
MODULE := geniezone

#Add service function
BL31_SOURCES += services/spd/tlkd/tlkd_helpers.S

#INCLUDE
PLAT_INCLUDES += -I${LOCAL_DIR}/plat/${MTK_SOC}

#Add your source code here
LOCAL_SRCS-$(CONFIG_MTK_GZ)       := ${LOCAL_DIR}/gz_boot_tag.c
LOCAL_SRCS-$(CONFIG_MTK_GZ_SMC)   += ${LOCAL_DIR}/gz_smc.c
LOCAL_SRCS-$(CONFIG_MTK_GZ_SMC)   += ${LOCAL_DIR}/gz_smc_to_el2.c
LOCAL_SRCS-$(CONFIG_MTK_GZ_SECIO) += ${LOCAL_DIR}/gz_secio.c
LOCAL_SRCS-$(CONFIG_MTK_GZ_SECIO) += ${LOCAL_DIR}/gz_secio_mmap.c

#Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
