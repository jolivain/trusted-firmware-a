#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#TODO: better solution?
set(HW_PLAT_CONFIG ${PROJECT_SOURCE_DIR}/configs/ConfigPlatFVP.cmake)
set(HW_PLAT_TARGET ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/platform.cmake)
set(HW_PLAT_IMAGE ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/image.cmake)

include(${HW_PLAT_CONFIG})
