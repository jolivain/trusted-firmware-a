#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${PHIT_HOB},1)

PHIT_HOB_SOURCES	+=	$(addprefix lib/hob/,	\
				hob.c)

INCLUDES	+=	-Iinclude/lib/hob

BL31_SOURCES	+=	$(PHIT_HOB_SOURCES)

endif	# PHIT_HOB

