#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_executable(tfa-bl1)

target_compile_definitions(tfa-bl1
    PRIVATE "IMAGE_BL1")

if(TFA_ARCH_STATE_AARCH64)
    target_compile_definitions(tfa-bl1
        PRIVATE "IMAGE_AT_EL3")
endif()

target_sources(tfa-bl1
    PRIVATE "bl1_main.c")

target_link_libraries(tfa-bl1
    PRIVATE tfa-platform-bl1
    PRIVATE tfa-common-platform-stack-setup
    PRIVATE tfa-lib-cpu-dsu-helpers
            tfa-lib-cpu-errata-reporting
            tfa-lib-cpu-helpers
            tfa-lib-el3-runtime-cm
            tfa-lib-fconf
            tfa-lib-fconf-dynamic)

add_subdirectory("${TFA_ARCH_STATE_LOWER_IDENT}")

target_link_libraries(tfa-bl1
    PRIVATE tfa-bl1-${TFA_ARCH_STATE_LOWER})

if(TFA_DISABLE_MTPMU)
    target_link_libraries(tfa-bl1
        PRIVATE tfa-lib-extension-mtpmu)
endif()

if(TFA_TRUSTED_BOARD_BOOT)
    target_sources(tfa-bl1
        PRIVATE "bl1_fwu.c")
endif()

arm_target_linker_script(
    TARGET tfa-bl1 SCRIPT "bl1.ld.S"
    PREPROCESSOR SUBTARGET tfa-bl1-lds LANGUAGE ASM)

set_property(TARGET tfa-bl1-lds APPEND
    PROPERTY COMPILE_DEFINITIONS "__LINKER__")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPIN_ON_BL1_EXIT

   This option introduces an infinite loop in BL1, and is disabled by default.
   This loop stops execution in BL1 just before handing over to BL31. At this
   point, all firmware images have been loaded into memory, and the MMU and
   caches are turned off. Refer to the :ref:`Debugging options` section for more
   details.
#]=======================================================================]

arm_config_option(
    NAME TFA_SPIN_ON_BL1_EXIT
    HELP "Loop infinitely prior to exiting BL1.")

target_compile_definitions(tfa-bl1
    PRIVATE "SPIN_ON_BL1_EXIT=$<BOOL:${TFA_SPIN_ON_BL1_EXIT}>")
