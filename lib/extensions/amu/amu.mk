#
# Copyright (c) 2021, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/fconf/fconf.mk

AMU_SOURCES	+=	lib/extensions/amu/${ARCH}/amu.c \
			lib/extensions/amu/${ARCH}/amu_helpers.S

ifeq (${ENABLE_AMU_FCONF},1)
        AMU_SOURCES	+=	lib/extensions/amu/amu_fconf.c \

        AMU_SOURCES	+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}
        AMU_SOURCES	+=	common/fdt_wrappers.c
endif
