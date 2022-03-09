#
# Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch64)
        $(error "Error: SPMC is only supported on aarch64.")
endif

SPMC_SOURCES	:=	$(addprefix services/std_svc/spm/spmc/,	\
			spmc_main.c				\
			spmc_setup.c				\
			logical_sp.c)

# Specify LP implementations separately for clarity.
SPMC_LP_SOURCES  := $(addprefix services/std_svc/spm/spmc/,	\
		    logical_sp_test.c)

SPMC_SOURCES += $(SPMC_LP_SOURCES)

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes

# Enable save and restore for non-secure timer register
NS_TIMER_SWITCH		:=	1
