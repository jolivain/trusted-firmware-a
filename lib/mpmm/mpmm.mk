#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/extensions/amu/amu.mk
include lib/fconf/fconf.mk

ifneq ($(ARCH),aarch64)
	$(error MPMM is only supported when building for AArch64)
endif

ifeq (${ENABLE_MPMM},1)
	ifneq (${ENABLE_AMU},1)
		$(error MPMM support requires AMU support (see `ENABLE_AMU`))
	endif
endif

MPMM_SOURCES	+=	lib/mpmm/mpmm.c \
			lib/mpmm/mpmm_fconf.c

MPMM_SOURCES	+=	${AMU_SOURCES} \
			${FCONF_SOURCES} ${FCONF_DYN_SOURCES}
