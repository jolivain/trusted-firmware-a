#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Compile time defines used by NXP platforms
ifneq ($(findstring lx2,$(PLAT)),)
  PLAT_DEF_OID := yes
else
ifneq (${FUSE_PROV_FILE},)
  PLAT_DEF_OID := yes
endif
endif

ifeq (${PLAT_DEF_OID},yes)
HOSTCCFLAGS += -DPLAT_DEF_OID -DPDEF_KEYS -DPDEF_CERTS -DPDEF_EXTS
$(eval $(call add_define, PLAT_DEF_OID))
$(eval $(call add_define, PDEF_KEYS))
$(eval $(call add_define, PDEF_CERTS))
$(eval $(call add_define, PDEF_EXTS))
INC_DIR += -I../../plat/nxp/common/fip_handler/common/

PDEF_CERT_TOOL_PATH		:=	../nxp/cert_create_helper
PLAT_INCLUDE			+=	-I${PDEF_CERT_TOOL_PATH}/include

OBJECTS				+=	${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_cert.o \
					${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_ext.o \
					${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_key.o
endif
