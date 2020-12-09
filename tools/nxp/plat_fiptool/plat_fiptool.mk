#
# Copyright (c) 2021, NXP. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq ($(findstring lx2,$(PLAT)),)
INCLUDE_MK	:= yes
endif

ifneq (${FUSE_PROV_FILE},)
INCLUDE_MK	:= yes
endif

ifeq (${INCLUDE_MK}, yes)
PLAT_DEF_UUID := yes
PLAT_DEF_OID := yes
PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/common/fip_handler/common
PLAT_DEF_UUID_CONFIG_FILE_PATH := ../nxp/plat_fiptool/

ifneq (${PLAT_DEF_UUID_OID_CONFIG_PATH},)
INCLUDE_PATHS += -I${PLAT_DEF_UUID_OID_CONFIG_PATH}
endif

ifeq (${PLAT_DEF_OID},yes)
HOSTCCFLAGS += -DPLAT_DEF_OID
endif

ifeq (${PLAT_DEF_UUID},yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
PLAT_OBJECTS += ${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o
endif

OBJECTS += ${PLAT_OBJECTS}

endif
