#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#TODO: better solution?
set(HW_PLAT_CONFIG ${PROJECT_SOURCE_DIR}/configs/ConfigPlatFVP.cmake)

set(HW_PLAT_DIR ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp)
set(HW_PLAT_TARGET ${HW_PLAT_DIR}/platform.cmake)
set(HW_PLAT_IMAGE ${HW_PLAT_DIR}/image.cmake)

include(${HW_PLAT_CONFIG})
