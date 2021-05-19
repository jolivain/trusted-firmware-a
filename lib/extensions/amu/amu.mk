#
# Copyright (c) 2021, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/fconf/fconf.mk

AMU_SOURCES	+=	lib/extensions/amu/${ARCH}/amu.c \
			lib/extensions/amu/${ARCH}/amu_helpers.S
