#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_add_src_param(NAME bl1 bl2 bl31 KEY ARCH SRC
	${TFA_ROOT_DIR}/plat/arm/soc/common/soc_css_security.c
)
