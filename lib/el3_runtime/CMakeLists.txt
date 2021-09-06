#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the context management library.
#

add_library(tfa-lib-el3-runtime-cm INTERFACE)

target_compile_definitions(tfa-lib-el3-runtime-cm
    INTERFACE "CTX_INCLUDE_EL2_REGS=$<BOOL:${TFA_CTX_INCLUDE_EL2_REGS}>"
              "CTX_INCLUDE_FPREGS=$<BOOL:${TFA_CTX_INCLUDE_FPREGS}>"
              "CTX_INCLUDE_MTE_REGS=$<BOOL:${TFA_CTX_INCLUDE_MTE_REGS}>"
              "CTX_INCLUDE_PAUTH_REGS=$<BOOL:${TFA_CTX_INCLUDE_PAUTH_REGS}>"
              "NS_TIMER_SWITCH=$<BOOL:${TFA_NS_TIMER_SWITCH}>")

target_include_directories(tfa-lib-el3-runtime-cm
    INTERFACE "${TFA_SOURCE_DIR}/include/lib/el3_runtime/${TFA_ARCH_STATE_LOWER}")

target_sources(tfa-lib-el3-runtime-cm
    INTERFACE "${TFA_ARCH_STATE_LOWER}/context_mgmt.c")

if(TFA_ARCH_STATE_AARCH64)
    target_sources(tfa-lib-el3-runtime-cm
        INTERFACE "${TFA_ARCH_STATE_LOWER}/context.S")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_AARCH32_REGS

   This option provides context save/restore operations for the AArch32 system
   registers. The option must not be enabled for AArch64-only platforms (that
   is, on hardware that does not implement AArch32, or at least not at EL1 and
   higher ELs).

   Enabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_CTX_INCLUDE_AARCH32_REGS
    HELP "Include AArch32 system registers in context switches?"
    DEFAULT TRUE)

target_compile_definitions(tfa-lib-el3-runtime-cm
    INTERFACE "CTX_INCLUDE_AARCH32_REGS=$<BOOL:${TFA_CTX_INCLUDE_AARCH32_REGS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_EL2_REGS

   This option provides context save/restore operations when entering/exiting an
   EL2 execution context. This is of primary interest when the Armv8.4-A Secure
   EL2 extension is implemented. This option is disabled by default, but is
   forcibly enabled if the Secure Payload is configured to be SPMD, and SPMD
   This option is forcibly enabled if :ref:`SPMD_SPM_AT_SEL2` is enabled.
#]=======================================================================]

if((TFA_SPMD STREQUAL "spmd") AND TFA_SPMD_SPM_AT_SEL2)
    arm_config_option_override(
        NAME TFA_CTX_INCLUDE_EL2_REGS
        FORCE TRUE)
endif()

arm_config_option(
    NAME TFA_CTX_INCLUDE_EL2_REGS
    HELP "Include floating point registers in context switches?")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_EL2_REGS

   This option provides context save/restore operations when entering/exiting an
   EL2 execution context. This is of primary interest when the Armv8.4-A Secure
   EL2 extension is implemented. This option is disabled by default, but is
   forcibly enabled if the Secure Payload is configured to be SPMD, and SPMD
   This option is forcibly enabled if :ref:`SPMD_SPM_AT_SEL2` is enabled.
#]=======================================================================]

if((TFA_SPMD STREQUAL "spmd") AND TFA_SPMD_SPM_AT_SEL2)
    arm_config_option_override(
        NAME TFA_CTX_INCLUDE_EL2_REGS
        FORCE TRUE)
endif()

arm_config_option(
    NAME TFA_CTX_INCLUDE_EL2_REGS
    HELP "Include floating point registers in context switches?")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_FPREGS

   When enabled, floating point registers will be included when saving and
   restoring the CPU context. Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_CTX_INCLUDE_FPREGS
    HELP "Include floating point registers in context switches?")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_MTE_REGS

   Enables Memory Tagging for the Secure world. This will cause the ARMv8.5-A
   Memory Tagging registers to be included when saving and restoring the CPU
   context as part of world switches. This is an experimental feature and is
   disabled by default.

   Note that Memory Tagging is enabled for the Non-secure world irrespective of
   the value of this flag if the CPU supports it.
#]=======================================================================]

arm_config_option(
    NAME TFA_CTX_INCLUDE_MTE_REGS
    HELP "Include memory tagging registers in context switches?"
    DEPENDS ${TFA_ARCH_VERSION} VERSION_GREATER_EQUAL 8.5)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_NEVE_REGS

   When enabled, this option will cause the Armv8.4-A nested virtualization
   registers to be saved/restored when entering/exiting an EL2 execution
   context.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_CTX_INCLUDE_NEVE_REGS
    HELP "Include Armv8.4-A nested virtualization registers in context switches?")

target_compile_definitions(tfa-common
    INTERFACE "CTX_INCLUDE_NEVE_REGS=$<BOOL:${TFA_CTX_INCLUDE_NEVE_REGS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_PAUTH_REGS

   Enables Pointer Authentication for the Secure world. This will cause the
   ARMv8.3-A Pointer Authentication registers to be included when saving and
   restoring the CPU context as part of world switches. This is an experimental
   feature and is disabled by default.

   Note that Pointer Authentication is enabled for the Non-secure world
   irrespective of the value of this flag if the CPU supports it.
#]=======================================================================]

arm_config_option(
    NAME TFA_CTX_INCLUDE_PAUTH_REGS
    HELP "Include pointer authentication registers in context switches?"
    DEPENDS ${TFA_ARCH_VERSION} VERSION_GREATER_EQUAL 8.3)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_NS_TIMER_SWITCH

   Enable save and restore of non-secure timer register contents upon world
   switch. Disabed by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_NS_TIMER_SWITCH
    HELP "Include non-secure timer registers in context switches?")
