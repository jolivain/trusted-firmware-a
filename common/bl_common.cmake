#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

#bl_common
stgt_add_src_param(NAME bl1 bl2 bl31 bl32 KEY ARCH SRC
	${TFA_ROOT_DIR}/common/bl_common.c
	${TFA_ROOT_DIR}/common/tf_log.c
	${TFA_ROOT_DIR}/common/@ARCH@/debug.S
	${TFA_ROOT_DIR}/drivers/console/multi_console.c
	${TFA_ROOT_DIR}/lib/@ARCH@/cache_helpers.S
	${TFA_ROOT_DIR}/lib/@ARCH@/misc_helpers.S
	${TFA_ROOT_DIR}/plat/common/plat_bl_common.c
	${TFA_ROOT_DIR}/plat/common/plat_log_common.c
	${TFA_ROOT_DIR}/plat/common/@ARCH@/plat_common.c
	${TFA_ROOT_DIR}/plat/common/@ARCH@/platform_helpers.S
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/popcountdi2.c
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/popcountsi2.c
	#TODO: armclang, ubsan, stack protector
)

stgt_add_src_cond(NAME bl1 bl2 bl31 bl32 KEY ARCH VAL aarch32 SRC
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/arm/aeabi_ldivmod.S
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/arm/aeabi_uldivmod.S
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/ctzdi2.c
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/divdi3.c
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/divmoddi4.c
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/lshrdi3.c
	${TFA_ROOT_DIR}/lib/compiler-rt/builtins/udivmoddi4.c
)

stgt_add_src_cond(NAME bl1 bl2 bl31 bl32 KEY ENABLE_BACKTRACE VAL 1 SRC
	${TFA_ROOT_DIR}/common/backtrace/backtrace.c
)
