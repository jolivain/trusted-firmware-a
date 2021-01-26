#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(TFAConfigOption)
include(TFATargetLinkerScript)

add_executable(bl1-test)

target_compile_definitions(bl1-test
    PRIVATE
        "IMAGE_BL1")

target_sources(bl1-test
    PRIVATE
        "bl1_main.c"
        "${TFA_ARCH}/bl1_arch_setup.c"
        "${TFA_ARCH}/bl1_context_mgmt.c"
        "${TFA_ARCH}/bl1_entrypoint.S"
        "${TFA_ARCH}/bl1_exceptions.S")

target_link_libraries(bl1-test
    PRIVATE
        common
        common-platform-bl1
        platform-bl1
        platform-stack-setup
        context-management
        cpu-helpers
        errata-reporting
        fconf
        fconf-dynamic)

if(TFA_DISABLE_MTPMU)
    target_link_libraries(bl1-test
        PRIVATE
            mtpmu)
endif()

if(TFA_TRUSTED_BOARD_BOOT)
    target_sources(bl1-test
        PRIVATE "bl1_fwu.c")
endif()

tfa_target_linker_script(bl1-test "bl1.ld.S")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPIN_ON_BL1_EXIT

   This option introduces an infinite loop in BL1, and is disabled by default.
   This loop stops execution in BL1 just before handing over to BL31. At this
   point, all firmware images have been loaded into memory, and the MMU and
   caches are turned off. Refer to the :ref:`Debugging options` section for more
   details.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SPIN_ON_BL1_EXIT
    HELP "Loop infinitely prior to exiting BL1.")

target_compile_definitions(bl1-test
    PRIVATE
        "SPIN_ON_BL1_EXIT=$<BOOL:${TFA_SPIN_ON_BL1_EXIT}>")
