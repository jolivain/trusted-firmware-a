#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${CSU_ADDED},)

CSU_ADDED		:= 1

CSU_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/csu

PLAT_INCLUDES		+= -I$(CSU_DRIVERS_PATH)

SECURITY_SOURCES	+= $(CSU_DRIVERS_PATH)/csu.c

endif
#------------------------------------------------
