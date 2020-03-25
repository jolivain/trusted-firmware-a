#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

SEC_DRIVERS_PATH	:=	drivers/nxp/fsl_sec

SEC_SOURCES +=  $(wildcard $(SEC_DRIVERS_PATH)/src/*.c)

PLAT_INCLUDES			+= -I$(SEC_DRIVERS_PATH)/include
