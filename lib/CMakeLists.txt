#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the architecture library. This changes quite a lot based on the
# architecture and the compiler chosen, but there's enough commonality that we
# can lump them into one aliased target.
#

add_subdirectory("${TFA_ARCH_STATE_LOWER_IDENT}" EXCLUDE_FROM_ALL)

#
# Pull in the remaining libraries, which maintain a bit more of a consistent
# directory structure than the architecture library.
#

add_subdirectory("cpus" EXCLUDE_FROM_ALL)
add_subdirectory("el3_runtime" EXCLUDE_FROM_ALL)
add_subdirectory("extensions" EXCLUDE_FROM_ALL)
add_subdirectory("fconf" EXCLUDE_FROM_ALL)
add_subdirectory("libc" EXCLUDE_FROM_ALL)
add_subdirectory("libfdt" EXCLUDE_FROM_ALL)
add_subdirectory("semihosting" EXCLUDE_FROM_ALL)
add_subdirectory("utils" EXCLUDE_FROM_ALL)

if(TFA_XLAT_TABLES_LIB_V1)
    add_subdirectory("xlat_tables" EXCLUDE_FROM_ALL)
else()
    add_subdirectory("xlat_tables_v2" EXCLUDE_FROM_ALL)
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_XLAT_TABLES_LIB_V1

   Compile TF-A with version 1 of the translation tables library instead of
   version 2.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_XLAT_TABLES_LIB_V1
    HELP "Use version 1 of the translation tables library?")

target_compile_definitions(tfa-lib-xlat-tables
    INTERFACE "ARM_XLAT_TABLES_LIB_V1=$<BOOL:${TFA_XLAT_TABLES_LIB_V1}>")
