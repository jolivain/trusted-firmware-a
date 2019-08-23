#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#TODO: better solution?
set(HW_PLAT_CONFIG ${TFA_ROOT_DIR}/configs/ConfigPlatJuno.cmake)
set(HW_PLAT_TARGET ${TFA_ROOT_DIR}/plat/arm/board/juno/platform.cmake)
#set(HW_PLAT_IMAGE ${TFA_ROOT_DIR}/plat/arm/board/juno/image.cmake)

find_package(fiptool REQUIRED)
find_package(dtc REQUIRED)
