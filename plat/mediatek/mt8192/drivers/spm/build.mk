#
# Copyright (c) from 2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable or disable spm feature
MT_SPM_FEATURE_SUPPORT = yes

# Enable or disable cirq restore
MT_SPM_CIRQ_FEATURE_SUPPORT = yes

# Enable or disable sspm sram
MT_SPMFW_SPM_SRAM_SLEEP_SUPPORT = no

# sspm notifier support
MT_SPM_SSPM_NOTIFIER_SUPPORT = yes

# spm trace support
MT_SPM_TRACE_SUPPORT = no

CUR_SPM_FOLDER = ${MTK_PLAT_SOC}/drivers/spm

# spm common files
PLAT_SPM_SOURCE_FILES_COMMON += ${CUR_SPM_FOLDER}/mt_spm.c			\
				${CUR_SPM_FOLDER}/mt_spm_internal.c		\
				${CUR_SPM_FOLDER}/mt_spm_vcorefs.c		\
				${CUR_SPM_FOLDER}/mt_spm_pmic_wrap.c		\
				${CUR_SPM_FOLDER}/mt_spm_conservation.c		\
				${CUR_SPM_FOLDER}/tracer/mt_spm_sysram.c	\


# spm platform dependcy files
PLAT_SPM_SOURCE_FILES += ${CUR_SPM_FOLDER}/mt_spm_cond.c		\
			 ${CUR_SPM_FOLDER}/mt_spm_suspend.c 		\
			 ${CUR_SPM_FOLDER}/mt_spm_idle.c		\
			 ${CUR_SPM_FOLDER}/mt_spm_dispatcher.c		\
			 ${CUR_SPM_FOLDER}/mt_spm_dbg_dispatcher.c	\
			 ${CUR_SPM_FOLDER}/constraints/mt_spm_rc_api.c \
			 ${CUR_SPM_FOLDER}/constraints/mt_spm_rc_dram.c \
			 ${CUR_SPM_FOLDER}/constraints/mt_spm_rc_syspll.c \
			 ${CUR_SPM_FOLDER}/constraints/mt_spm_rc_bus26m.c \
			 ${CUR_SPM_FOLDER}/constraints/mt_spm_rc_cpu_buck_ldo.c



ifeq (${MT_SPM_FEATURE_SUPPORT},no)
PLAT_SPM_DEBUG_CFLAGS += -DATF_PLAT_SPM_UNSUPPORT
BL31_MT_LPM_PLAT_SPM_SOURCE_FILES += ${PLAT_SPM_SOURCE_FILES_COMMON}
else
BL31_MT_LPM_PLAT_SPM_SOURCE_FILES += ${PLAT_SPM_SOURCE_FILES_COMMON} \
				     ${PLAT_SPM_SOURCE_FILES}
endif

ifeq (${MT_SPM_CIRQ_FEATURE_SUPPORT},no)
PLAT_SPM_DEBUG_CFLAGS += -DATF_PLAT_CIRQ_UNSUPPORT
endif


ifeq (${MT_SPMFW_SPM_SRAM_SLEEP_SUPPORT},no)
PLAT_SPM_DEBUG_CFLAGS += -DATF_PLAT_SPM_SRAM_SLP_UNSUPPORT
endif

ifeq (${MT_SPM_SSPM_NOTIFIER_SUPPORT},no)
PLAT_SPM_DEBUG_CFLAGS += -DATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
else
BL31_MT_LPM_PLAT_SPM_SOURCE_FILES += ${CUR_SPM_FOLDER}/notifier/mt_spm_sspm_notifier.c
endif

ifeq (${MT_SPM_TRACE_SUPPORT},no)
PLAT_SPM_DEBUG_CFLAGS += -DATF_PLAT_SPM_TRACE_UNSUPPORT
endif

$(info --------------------------------------)
$(info SPM build flags: ${PLAT_SPM_DEBUG_CFLAGS})
$(info SPM build files: ${BL31_MT_LPM_PLAT_SPM_SOURCE_FILES})
$(info --------------------------------------)


# Common makefile for platform.mk, It isn't need to modified usually
PLAT_INCLUDES += ${PLAT_SPM_DEBUG_CFLAGS} \
		-I${CUR_SPM_FOLDER}/ \
		-I${CUR_SPM_FOLDER}/constraints/ \
		-I${CUR_SPM_FOLDER}/notifier/ \
		-I${CUR_SPM_FOLDER}/tracer/


PLAT_BL_COMMON_SOURCES += ${BL31_MT_LPM_PLAT_SPM_SOURCE_FILES}


