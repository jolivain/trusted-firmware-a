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

stgt_add_src_param(NAME bl1 bl2 bl31 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/plat/arm/soc/common/soc_css_security.c
)
