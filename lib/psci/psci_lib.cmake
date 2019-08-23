#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIR not defined")
endif()

# Include framework files
include(Common/STGT)

stgt_add_src_param(NAME bl31 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/lib/el3_runtime/cpu_data_array.c
	${PROJECT_SOURCE_DIR}/lib/el3_runtime/@ARCH@/cpu_data.S
	${PROJECT_SOURCE_DIR}/lib/el3_runtime/@ARCH@/context_mgmt.c
	${PROJECT_SOURCE_DIR}/lib/cpus/@ARCH@/cpu_helpers.S
	${PROJECT_SOURCE_DIR}/lib/cpus/errata_report.c
	${PROJECT_SOURCE_DIR}/lib/locks/exclusive/@ARCH@/spinlock.S
)

stgt_add_src_param(NAME bl31 KEY ARCH SRC
	${CMAKE_CURRENT_LIST_DIR}/psci_off.c
	${CMAKE_CURRENT_LIST_DIR}/psci_on.c
	${CMAKE_CURRENT_LIST_DIR}/psci_suspend.c
	${CMAKE_CURRENT_LIST_DIR}/psci_common.c
	${CMAKE_CURRENT_LIST_DIR}/psci_main.c
	${CMAKE_CURRENT_LIST_DIR}/psci_setup.c
	${CMAKE_CURRENT_LIST_DIR}/psci_system_off.c
	${CMAKE_CURRENT_LIST_DIR}/psci_mem_protect.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/psci_helpers.S
)

stgt_add_src_cond(NAME bl31 KEY ARCH VAL "aarch64" SRC
	${PROJECT_SOURCE_DIR}/lib/el3_runtime/aarch64/context.S
)

stgt_add_src_cond(NAME bl31 KEY USE_COHERENT_MEM VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/locks/bakery/bakery_lock_coherent.c
)
stgt_add_src_cond(NAME bl31 KEY USE_COHERENT_MEM VAL 0 SRC
	${PROJECT_SOURCE_DIR}/lib/locks/bakery/bakery_lock_normal.c
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_PSCI_STAT VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/psci/psci_stat.c
)
