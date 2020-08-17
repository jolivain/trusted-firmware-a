#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBFDT_SRCS	:=	$(addprefix lib/libfdt/,	\
			fdt.c				\
			fdt_addresses.c			\
			fdt_empty_tree.c		\
			fdt_ro.c			\
			fdt_rw.c			\
			fdt_strerror.c			\
			fdt_sw.c			\
			fdt_wip.c)			\

INCLUDES	+=	-Iinclude/lib/libfdt

# libfdt has known issues with comparing types of different signedness.
# Fixes are not trivial, so we should wait for a proper upstream solution.
# More details: https://www.spinics.net/lists/devicetree-compiler/msg02953.html
CFLAGS		+=	-Wno-sign-compare

$(eval $(call MAKE_LIB,fdt))
