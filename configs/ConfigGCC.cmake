#-------------------------------------------------------------------------------
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

group_new(NAME compiler)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	group_add(NAME compiler TYPE CFLAG KEY -g)
	group_add(NAME compiler TYPE ASFLAG KEY -g)
	group_add(NAME compiler TYPE ASFLAG KEY -Wa,--gdwarf-2)
endif()

group_add(NAME compiler TYPE CFLAG KEY -Os)
group_add(NAME compiler TYPE CFLAG KEY -march VAL armv8-a)
group_add(NAME compiler TYPE CFLAG KEY -mgeneral-regs-only)
group_add(NAME compiler TYPE CFLAG KEY -mstrict-align)
group_add(NAME compiler TYPE CFLAG KEY -nostdinc)
group_add(NAME compiler TYPE CFLAG KEY -ffreestanding)
group_add(NAME compiler TYPE CFLAG KEY -fno-builtin)
group_add(NAME compiler TYPE CFLAG KEY -std VAL gnu99)
group_add(NAME compiler TYPE CFLAG KEY -ffunction-sections)
group_add(NAME compiler TYPE CFLAG KEY -fdata-sections)
group_add(NAME compiler TYPE CFLAG KEY -fno-omit-frame-pointer) #TODO: from backtrace

group_add(NAME compiler TYPE CFLAG KEY -Wall)
group_add(NAME compiler TYPE CFLAG KEY -Wdisabled-optimization)
group_add(NAME compiler TYPE CFLAG KEY -Werror)
group_add(NAME compiler TYPE CFLAG KEY -Wmaybe-uninitialized)
group_add(NAME compiler TYPE CFLAG KEY -Wmissing-include-dirs)
group_add(NAME compiler TYPE CFLAG KEY -Wno-error=cpp)
group_add(NAME compiler TYPE CFLAG KEY -Wno-error=deprecated-declarations)
group_add(NAME compiler TYPE CFLAG KEY -Wno-unused-parameter)
group_add(NAME compiler TYPE CFLAG KEY -Wpacked-bitfield-compat)
group_add(NAME compiler TYPE CFLAG KEY -Wshift-overflow=2)
group_add(NAME compiler TYPE CFLAG KEY -Wunused)
group_add(NAME compiler TYPE CFLAG KEY -Wunused-but-set-variable)
group_add(NAME compiler TYPE CFLAG KEY -Wvla)

group_add(NAME compiler TYPE ASFLAG KEY -ffreestanding)
group_add(NAME compiler TYPE ASFLAG KEY -Wa,--fatal-warnings)

group_add(NAME compiler TYPE LDFLAG KEY -nostdlib)
group_add(NAME compiler TYPE LDFLAG KEY -Wl,-O1)
group_add(NAME compiler TYPE LDFLAG KEY -Wl,--gc-sections)
