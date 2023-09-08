#
# Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

################################################################################
# Include Makefile for build flags which don't follow the specified naming
# convention. Warnings are given to encourage updating the build flag names,
# and the new build flags are automatically set.
################################################################################

ifneq (${ENABLE_SYS_REG_TRACE_FOR_NS},)
$(warning As part of an effort to unify the build flag naming \
	conventions, ENABLE_SYS_REG_TRACE_FOR_NS will soon be deprecated. \
	The updated version of this flag is ENABLE_SYS_REG_TRACE.")
ENABLE_SYS_REG_TRACE	:= ENABLE_SYS_REG_TRACE_FOR_NS
endif #(ENABLE_SYS_REG_TRACE_FOR_NS)
