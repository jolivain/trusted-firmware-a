#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIRnot defined")
endif()

# TODO: Skip building BL1, BL2 and BL2U if RESET_TO_SP_MIN flag is set.


stgt_add_src(NAME bl32 SRC
	${PROJECT_SOURCE_DIR}/plat/arm/common/arm_pm.c
	${PROJECT_SOURCE_DIR}/plat/arm/common/arm_topology.c
	${PROJECT_SOURCE_DIR}/plat/arm/common/sp_min/arm_sp_min_setup.c
	${PROJECT_SOURCE_DIR}/plat/common/aarch32/platform_mp_stack.S
	${PROJECT_SOURCE_DIR}/plat/common/plat_psci_common.c
)
