#-------------------------------------------------------------------------------
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#TODO: better solution?
set(HW_PLAT_CONFIG ${PROJECT_SOURCE_DIR}/configs/ConfigPlatJuno.cmake)
set(HW_PLAT_TARGET ${PROJECT_SOURCE_DIR}/plat/arm/board/juno/platform.cmake)
#set(HW_PLAT_IMAGE ${PROJECT_SOURCE_DIR}/plat/arm/board/juno/image.cmake)

find_package(FIPTool REQUIRED)
find_package(dtc REQUIRED)
