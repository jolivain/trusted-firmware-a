#
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20)

#
# Ensure our own CMake modules can be loaded.
#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

include(ArmConfigOption)
include(ArmExpand)
include(ArmPreprocessSource)
include(ArmTargetLinkerScript)

include(TFAAddBuildMessage)
include(TFAJsonUtilities)
include(TFALegacyOption)
include(TFAMetadata)
include(TFAParseArchVersion)

include(CheckCCompilerFlag)
include(CheckSymbolExists)

#
# Set up the configuration types for both single and multi-configuration
# generators.
#

set(config-types "Debug" "RelWithDebInfo" "MinSizeRel" "Release")
set(default-config "MinSizeRel")

get_property(multi-config GLOBAL PROPERTY "GENERATOR_IS_MULTI_CONFIG")

if(multi-config)
    arm_config_option(
        NAME CMAKE_CONFIGURATION_TYPES HIDDEN
        HELP "Multi-generator configuration types."
        DEFAULT ${config-types})

    arm_config_option(
        NAME CMAKE_DEFAULT_BUILD_TYPE HIDDEN
        HELP "Default multi-generator configuration type."
        DEFAULT "${default-config}")
else()
    arm_config_option(
        NAME CMAKE_BUILD_TYPE
        HELP "Build type."
        STRINGS ${config-types}
        DEFAULT ${default-config}
        FORCE NOT CMAKE_BUILD_TYPE)
endif()

#
# Retrieve the list of platforms from the global metadata file and present them
# to the user.
#

tfa_platforms(platforms)

arm_assert(
    CONDITION platforms
    MESSAGE "No platforms defined!")

arm_config_option(
    NAME TFA_PLATFORM
    HELP "Platform to build."
    STRINGS ${platforms})

tfa_platform_path(TFA_PLATFORM_SOURCE_DIR
    PLATFORM "${TFA_PLATFORM}")

#
# Because the platform's source directory might have come from outside, CMake
# may be unable to derive the platform's binary directory automatically. As an
# alternative, we'll use the platform's target name as its binary directory.
#

tfa_platform_target(target
    PLATFORM "${TFA_PLATFORM}")

set(TFA_PLATFORM_BINARY_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/${target}")

#
# Configure the Arm architecture to target. In the legacy build system the
# version parts were configured through `ARM_ARCH_MAJOR` and `ARM_ARCH_MINOR`.
# Here, we instead accept a string in the branded format - "ArmvX.Y-Z" - and
# derive the versions from that.
#

tfa_platform_architectures(architectures
    PLATFORM ${TFA_PLATFORM})

tfa_legacy_option(
    OLD ARM_ARCH_MAJOR ARM_ARCH_MINOR
    NEW TFA_ARCH)

arm_config_option(
    NAME TFA_ARCH
    HELP "Arm architecture to target."
    STRINGS ${architectures})

tfa_parse_arch_version(
    VERSION "${TFA_ARCH}"
    MAJOR TFA_ARCH_VERSION_MAJOR
    MINOR TFA_ARCH_VERSION_MINOR
    CLASS TFA_ARCH_VERSION_CLASS)

set(TFA_ARCH_VERSION "${TFA_ARCH_VERSION_MAJOR}.${TFA_ARCH_VERSION_MINOR}")

string(TOUPPER "${TFA_ARCH}" TFA_ARCH_UPPER)
string(MAKE_C_IDENTIFIER "${TFA_ARCH_UPPER}" TFA_ARCH_UPPER_IDENT)
string(TOLOWER "${TFA_ARCH}" TFA_ARCH_LOWER)
string(MAKE_C_IDENTIFIER "${TFA_ARCH_LOWER}" TFA_ARCH_LOWER_IDENT)

arm_config_option_override(
    NAME TFA_ARCH_${TFA_ARCH_UPPER_IDENT}
    VALUE TRUE)

#
# Configure the instruction set state. In the legacy build system this was done
# through `ARCH`, which took `aarch32` or `aarch64` regardless of the platform.
# We do something similar here, except we ask the platform to provide the list
# of architectures it supports if using an ISA supporting multiple instruction
# set states.
#

tfa_platform_architecture_states(states
    PLATFORM "${TFA_PLATFORM}"
    ARCHITECTURE "${TFA_ARCH}")

tfa_legacy_option(
    OLD ARCH
    NEW TFA_ARCH_STATE)

arm_config_option(
    NAME TFA_ARCH_STATE
    HELP "Arm instruction set state."
    STRINGS ${states})

string(TOUPPER ${TFA_ARCH_STATE} TFA_ARCH_STATE_UPPER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_STATE_UPPER} TFA_ARCH_STATE_UPPER_IDENT)
string(TOLOWER ${TFA_ARCH_STATE} TFA_ARCH_STATE_LOWER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_STATE_LOWER} TFA_ARCH_STATE_LOWER_IDENT)

arm_config_option_override(
    NAME TFA_ARCH_STATE_${TFA_ARCH_STATE_UPPER_IDENT}
    VALUE TRUE)

#
# Create convenience variables for every architecture and state described by the
# metadata. This allows build system developers to use short-hand variable names
# for basic boolean operations on the current architecture, instead of relying
# on string comparisons.
#

tfa_architectures(architectures)

foreach(architecture IN LISTS architectures)
    string(MAKE_C_IDENTIFIER ${architecture} architecture-ident)
    string(TOUPPER ${architecture-ident} architecture-ident)

    arm_config_option(
        NAME TFA_ARCH_${architecture-ident} HIDDEN
        HELP "Is the ${architecture} architecture in use?"
        DEFAULT FALSE)

    tfa_architecture_states(states
        ARCHITECTURE "${architecture}")

    foreach(state IN LISTS states)
        string(MAKE_C_IDENTIFIER ${state} state-ident)
        string(TOUPPER ${state-ident} state-ident)

        arm_config_option(
            NAME TFA_ARCH_STATE_${state-ident} HIDDEN
            HELP "Is the ${architecture} ${state} state in use?"
            DEFAULT FALSE)
    endforeach()
endforeach()

#
# Enable Thumb code generation on AArch32. This was previously done by giving
# either `A32` or `T32` to `AARCH32_INSTRUCTION_SET`. This is now a boolean on
# whether or not we build with Thumb code generation, visible only if we're
# building for AArch32.
#

tfa_legacy_option(
    OLD AARCH32_INSTRUCTION_SET
    NEW TFA_ARCH_STATE_AARCH32_THUMB)

arm_config_option(
    NAME TFA_ARCH_STATE_AARCH32_THUMB
    HELP "Generate code for the Thumb instruction set."
    DEFAULT TRUE
    DEPENDS TFA_ARCH_STATE_AARCH32
    ELSE FALSE)

#
# If no toolchain file has been provided through the default CMake mechanism, we
# use our own mechanism for toolchain selection, with the intention of making it
# easier for users to specify a valid toolchain file.
#
# Rather than ask the user to provide a toolchain file, we instead glob for
# toolchain files in the `toolchains` directory of the repository. The filename
# of each toolchain file becomes a value we accept in the `TFA_TOOLCHAINS`
# configuration option.
#

set(toolchains "")
file(GLOB toolchain-files "toolchains/*.cmake")

foreach(toolchain-file IN LISTS toolchain-files)
    cmake_path(GET toolchain-file STEM toolchain)
    list(APPEND toolchains "${toolchain}")
endforeach()

if(DEFINED TFA_TOOLCHAIN_OLD)
    arm_config_option_override(
        NAME TFA_TOOLCHAIN
        VALUE "${TFA_TOOLCHAIN_OLD}")
endif()

arm_config_option(
    NAME TFA_TOOLCHAIN
    HELP "Toolchain name."
    STRINGS ${toolchains}
    DEFAULT "GNU")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    arm_config_option_override(
        NAME CMAKE_TOOLCHAIN_FILE
        VALUE "${CMAKE_CURRENT_LIST_DIR}/toolchains/${TFA_TOOLCHAIN}.cmake")
endif()

#
# Allow the user to configure the toolchain file. This uses the standard CMake
# mechanism for describing the toolchain.
#

if(DEFINED CMAKE_TOOLCHAIN_FILE_OLD)
    arm_config_option_override(
        NAME CMAKE_TOOLCHAIN_FILE
        VALUE "${CMAKE_TOOLCHAIN_FILE_OLD}")
endif()

arm_config_option(
    NAME CMAKE_TOOLCHAIN_FILE
    HELP "Path to the CMake toolchain file."
    TYPE FILEPATH)

arm_assert(
    CONDITION EXISTS "${CMAKE_TOOLCHAIN_FILE}"
    MESSAGE "Couldn't find toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

#
# We're done with very early setup, so we can now create the project. This will
# do some of the automatic compiler detection, which we need for setting up
# further configuration options.
#
# Note that this creates the following version variables:
#
# - `TFA_VERSION`
# - `TFA_VERSION_MAJOR`
# - `TFA_VERSION_MINOR`
# - `TFA_VERSION_PATCH`
# - `TFA_VERSION_TWEAK`
#
# Also, these directory variables:
#
# - `TFA_SOURCE_DIR`
# - `TFA_BINARY_DIR`
#
# Don't swap `C` and `ASM`. Per the CMake documentation:
#
# > If enabling `ASM`, list it last so that CMake can check whether compilers
# > for other languages like `C` work for assembly too.
#

project(TFA VERSION 2.5 LANGUAGES C ASM)

#
# Create the common library target. This target pulls together the various
# libraries that are common to virtually every component, and applies
# definitions for global configuration options.
#

add_library(tfa-common INTERFACE)

target_compile_definitions(tfa-common
    INTERFACE "ARM_ARCH_MAJOR=${TFA_ARCH_VERSION_MAJOR}"
              "ARM_ARCH_MINOR=${TFA_ARCH_VERSION_MINOR}"
    INTERFACE "DEBUG=$<CONFIG:Debug>"
    INTERFACE "ENABLE_ASSERTIONS=!NDEBUG"
    INTERFACE "PLAT_${TFA_PLATFORM_LOWER_IDENT}")

target_include_directories(tfa-common
    INTERFACE "include")

target_link_libraries(tfa-common
    INTERFACE tfa-lib-arch
              tfa-lib-c)

#
# Generate the build message source file and attach it to the common sources.
#

tfa_add_build_message(
    TARGETS tfa-common)

#
# Pull in the platform targets. Run this first so that the platform can provide
# overrides for configuration options defined later.
#

add_subdirectory("${TFA_PLATFORM_SOURCE_DIR}" "${TFA_PLATFORM_BINARY_DIR}"
    EXCLUDE_FROM_ALL)

#
# Pull in the list files representing their individual directories.
#

add_subdirectory("common" EXCLUDE_FROM_ALL)
add_subdirectory("drivers" EXCLUDE_FROM_ALL)
add_subdirectory("lib" EXCLUDE_FROM_ALL)
add_subdirectory("plat" EXCLUDE_FROM_ALL)

#
# Pull in the bootloader image targets.
#

add_subdirectory("bl1" EXCLUDE_FROM_ALL)

#
# Allow BL2 to be started at EL3.
#

arm_config_option(
    NAME TFA_BL2_AT_EL3
    HELP "Enable the use of BL2 at EL3.")

tfa_legacy_option(
    OLD BL2_AT_EL3
    NEW TFA_BL2_AT_EL3)

target_compile_definitions(tfa-common
    INTERFACE "BL2_AT_EL3=$<BOOL:${TFA_BL2_AT_EL3}>")

if(TFA_BL2_AT_EL3)
    message(WARNING "The option `TFA_BL2_AT_EL3` is incomplete!")
endif()

#
# Enable execution of BL2 from XIP memory.
#

arm_config_option(
    NAME TFA_BL2_IN_XIP_MEM
    HELP "Execute BL2 from eXecute In Place (XIP) memory."
    DEPENDS TFA_BL2_AT_EL3)

tfa_legacy_option(
    OLD BL2_IN_XIP_MEM)

target_compile_definitions(tfa-common
    INTERFACE "BL2_IN_XIP_MEM=$<BOOL:${TFA_BL2_IN_XIP_MEM}>")

if(TFA_BL2_IN_XIP)
    message(WARNING "The option `TFA_BL2_IN_XIP` is incomplete!")
endif()

#
# Handle Armv8.3-A Pointer Authentication and Armv8.5-A Branch Target
# Identification support through compiler flags directly. We can't know the
# flags and level of support for every compiler, so by default we just offer the
# flags we do know about and then the user can choose their own if they so wish.
#
# In the legacy build system we would set the values of the `ENABLE_BTI` and
# `ENABLE_PAUTH` definitions based on some prior knowledge of the build flags.
# We'll diverge from that here, and instead use the ACLE definitions that the
# compiler should set in response to these flags being enabled.
#

set(bp-options)

list(APPEND bp-options "-mbranch-protection=pac-ret")
list(APPEND bp-options "-mbranch-protection=pac-ret+leaf")
list(APPEND bp-options "-mbranch-protection=bti")
list(APPEND bp-options "-mbranch-protection=pac-ret+bti")
list(APPEND bp-options "-mbranch-protection=pac-ret+leaf+bti")

foreach(option IN LISTS bp-options)
    check_c_compiler_flag("${option}" supported)

    if(NOT supported)
        list(REMOVE_ITEM bp-options "${option}")
    endif()
endforeach()

arm_config_option(
    NAME TFA_BRANCH_PROTECTION_FLAGS
    HELP "Additional branch protection compiler flags."
    DEPENDS TFA_ARCH VERSION_GREATER_EQUAL 8.3
    STRINGS "${bp-options}" FREEFORM
    DEFAULT "")

target_compile_options(tfa-common
    INTERFACE ${TFA_BRANCH_PROTECTION_FLAGS})

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_BTI=__ARM_FEATURE_BTI_DEFAULT"
              "ENABLE_PAUTH=(__ARM_FEATURE_PAC_DEFAULT != 0)")

try_compile(have_feat_pauth_bkey "/tmp/testme"
    "${TFA_SOURCE_DIR}/cmake/Checks/have_feat_pauth_bkey.c"
    COMPILE_DEFINITIONS "${TFA_BRANCH_PROTECTION_FLAGS}")

if(have_feat_pauth_bkey)
    message(FATAL_ERROR
        "Sorry, pointer authentication using the B-key is not supported!")
endif()

#
# Optimize platforms that release only a single CPU out of reset.
#

arm_config_option(
    NAME TFA_COLD_BOOT_SINGLE_CPU
    HELP "Does this platform cold boot on a single CPU?")

target_compile_definitions(tfa-common
    INTERFACE "COLD_BOOT_SINGLE_CPU=$<BOOL:${TFA_COLD_BOOT_SINGLE_CPU}>")

#
# Enable crash reporting in BL31.
#
# Because we need to handle multi-config generators, this is a bit more involved
# than just `if(DEBUG)`. Instead, we have a configuration option for every
# build mode, and then we use a generator expression to pull in the right one
# for each generator target.
#

foreach(config IN LISTS config_types)
    set(default FALSE)

    if(config STREQUAL "Debug")
        set(default TRUE)
    endif()

    arm_config_option(
        NAME TFA_CRASH_REPORTING_${config}
        HELP "Enable crash reporting in BL31 (${config})?"
        DEFAULT "${default}")

    target_compile_definitions(tfa-common
        INTERFACE "$<$<CONFIG:${config}>:CRASH_REPORTING=$<BOOL:${TFA_CRASH_REPORTING_${config}}>>")
endforeach()

#
# Enable firmware decryption support.
#

arm_config_option(
    NAME TFA_DECRYPTION_SUPPORT
    HELP "Firmware decryption algorithm."
    DEPENDS TFA_TRUSTED_BOARD_BOOT
    STRINGS "AesGcm" "None"
    DEFAULT "None")

if(TFA_DECRYPTION_SUPPORT STREQUAL "AesGcm")
    set(decryption_support "aes_gcm")
else()
    set(decryption_support "")
endif()

target_compile_definitions(tfa-common
    INTERFACE "DECRYPTION_SUPPORT_${decryption_support}")

if(NOT TFA_DECRYPTION_SUPPORT STREQUAL "None")
    message(WARNING "The option `TFA_DECRYPTION_SUPPORT` is incomplete!")
endif()

#
# Allow disabling of `FEAT_MTPMU`.
#

arm_config_option(
    NAME TFA_DISABLE_MTPMU
    HELP "Disable the mulithreaded PMU extension?"
    DEPENDS TFA_ARCH VERSION_GREATER_EQUAL 8.6)

target_compile_definitions(tfa-common
    INTERFACE "DISABLE_MTPMU=$<BOOL:${TFA_DISABLE_MTPMU}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_AMU

   This option enables Activity Monitor Unit extensions. This is an optional
   architectural feature available on Armv8.4-A onwards. Some Armv8.2-A
   implementations also implement an AMU and this option can be used to enable
   this feature on those systems as well.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENABLE_AMU
    HELP "Enable Armv8.4-A Activity Monitor Unit extensions?")

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_AMU=$<BOOL:${TFA_ENABLE_AMU}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_MPAM_FOR_LOWER_ELS

   Enables MPAM feature in ELs lower than EL3. MPAM is an optional Armv8.4-a
   extension that enables various memory system components and resources to
   define partitions; software running at various ELs can assign themselves to a
   desired partition to control their performance aspects.

   When this option is enabled, EL3 allows lower ELs to access their own MPAM
   registers without trapping into EL3. This option doesn't make use of
   partitioning in EL3, however. Platform initialization code should configure
   and use partitions in EL3 as required.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENABLE_MPAM_FOR_LOWER_ELS
    HELP "Enable MPAM in ELs lower than EL3?")

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_MPAM_FOR_LOWER_ELS=$<BOOL:${TFA_ENABLE_MPAM_FOR_LOWER_ELS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_PIE

   Enable Position Independent Executable (PIE) support within generic code in
   TF-A. This option is currently only supported in BL2 when running in EL3,
   BL31, and BL32 (TSP).

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENABLE_PIE
    HELP "Build position-independent bootloader images (where possible)?")

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_PIE=$<BOOL:${TFA_ENABLE_PIE}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_PSCI_STAT

   Enables support for the optional PSCI functions ``PSCI_STAT_RESIDENCY`` and
   ``PSCI_STAT_COUNT``. In the absence of an alternative statistics collection
   backend, :ref:``TFA_ENABLE_PMF`` must be enabled. If :ref:``TFA_ENABLE_PMF``
   is enabled, the residency statistics are tracked in software.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENABLE_PSCI_STAT
    HELP "Enable support for PSCI statistics tracking.")

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_PSCI_STAT=$<BOOL:${TFA_ENABLE_PSCI_STAT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_RUNTIME_INSTRUMENTATION

   Enables runtime instrumentation, which injects timestamp collection points
   into TF-A to allow runtime performance to be measured. Currently, only PSCI
   is instrumented.

   Disabled by default. Forcibly disabled unless the Performance Monitoring
   Framework (PMF) is enabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENABLE_RUNTIME_INSTRUMENTATION
    HELP "Enable runtime instrumentation."
    DEPENDS TFA_ENABLE_PMF)

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_RUNTIME_INSTRUMENTATION=$<BOOL:${TFA_ENABLE_RUNTIME_INSTRUMENTATION}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_PMF

   Enables support for the Performance Measurement Framework (PMF).

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENABLE_PMF
    HELP "Enable the Performance Measurement Framework?"
    DEFAULT ${TFA_ENABLE_RUNTIME_INSTRUMENTATION})

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_PMF=$<BOOL:${TFA_ENABLE_PMF}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_SPE_FOR_LOWER_ELS

   Enable the Armv8.2-A Statistical Profiling Extension.

   Enabled by default. Forcibly disabled if not building for AArch64.
#]=======================================================================]

if(TFA_ARCH_STATE_AARCH32)
    arm_config_option_override(
        NAME TFA_ENABLE_SPE_FOR_LOWER_ELS
        FORCE FALSE)
endif()

arm_config_option(
    NAME TFA_ENABLE_SPE_FOR_LOWER_ELS
    HELP "Enable Armv8.2-A Statistical Profiling extensions?"
    DEFAULT TRUE)

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_SPE_FOR_LOWER_ELS=$<BOOL:${TFA_ENABLE_SPE_FOR_LOWER_ELS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_SVE_FOR_NS

   Enables Scalable Vector Extension (SVE) for the Normal world only. SVE is an
   optional architectural feature for AArch64. Note that when SVE is enabled for
   the Normal world, access to SIMD and floating-point functionality from the
   Secure world is disabled. This is to avoid corruption of the Normal world
   data in the Z-registers, which are aliased by the SIMD and FP registers.

   Enabled by default. Forcibly disabled if :ref:``TFA_CTX_INCLUDE_FPREGS`` is
   enabled, or if building for AArch32.
#]=======================================================================]

if(TFA_ARCH_STATE_AARCH32 OR TFA_CTX_INCLUDE_FPREGS)
    arm_config_option_override(
        NAME TFA_ENABLE_SVE_FOR_NS
        FORCE FALSE)
endif()

arm_config_option(
    NAME TFA_ENABLE_SVE_FOR_NS
    HELP "Enable the Armv8-A Scalable Vector Extension in the Normal world?"
    DEFAULT TRUE)

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_SVE_FOR_NS=$<BOOL:${TFA_ENABLE_SVE_FOR_NS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_STACK_PROTECTOR

   Additional stack protection flags, which are passed directly to the compiler.

   If stack protection flags are provided, the platform must implemented the
   ``plat_get_stack_protector_canary()`` function.

   Defaults to nothing.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENABLE_STACK_PROTECTOR
    HELP "Stack protection compiler flags."
    TYPE STRING)

if(TFA_ENABLE_STACK_PROTECTOR)
    target_compile_options(tfa-common
        INTERFACE "${TFA_ENABLE_STACK_PROTECTOR}")
endif()

target_compile_definitions(tfa-common
    INTERFACE "STACK_PROTECTOR_ENABLED=$<BOOL:${TFA_ENABLE_STACK_PROTECTOR}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENCRYPT_BL31

   Enables encryption of the BL31 firmware image.

   Disabled by default. Forcibly disabled if :ref:``TFA_DECRYPTION_SUPPORT`` is
   ``None``.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENCRYPT_BL31
    HELP "Enable encryption of BL31?"
    DEPENDS TFA_DECRYPTION_SUPPORT)

target_compile_definitions(tfa-common
    INTERFACE "ENCRYPT_BL31=$<BOOL:${TFA_ENCRYPT_BL31}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENCRYPT_BL32

   Enables encryption of the Secure BL32 firmware image.

   Disabled by default. Forcibly disabled if :ref:``TFA_DECRYPTION_SUPPORT`` is
   ``None``.
#]=======================================================================]

arm_config_option(
    NAME TFA_ENCRYPT_BL32
    HELP "Enable encryption of BL32?"
    DEPENDS TFA_DECRYPTION_SUPPORT)

target_compile_definitions(tfa-common
    INTERFACE "ENCRYPT_BL32=$<BOOL:${TFA_ENCRYPT_BL32}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERROR_DEPRECATED

   This option decides whether to treat the usage of deprecated platform APIs,
   helper functions or drivers within TF-A as an error.

    Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERROR_DEPRECATED
    HELP "Error on usage of deprecated APIs?")

target_compile_definitions(tfa-common
    INTERFACE "ERROR_DEPRECATED=$<BOOL:${TFA_ERROR_DEPRECATED}>")

if(TFA_ERROR_DEPRECATED)
    set(gnu_no_deprecated "-Wno-error=deprecated-declarations")
    set(gnu_no_cpp "-Wno-error=cpp")

    check_c_compiler_flag("${gnu_no_deprecated}" have_gnu_no_deprecated)
    check_c_compiler_flag("${gnu_no_cpp}" have_gnu_no_cpp)

    if(have_gnu_no_deprecated)
        target_compile_options(tfa-common INTERFACE "${gnu_no_deprecated}")
    endif()

    if(have_gnu_no_cpp)
        target_compile_options(tfa-common INTERFACE "${gnu_no_cpp}")
    endif()
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_EL3_EXCEPTION_HANDLING

   Enables handling of exceptions targeted at EL3. When disabled, no exceptions
   are expected or handled at EL3, and a panic will result.

   Disabled by default. Forcibly disabled if not building for AArch64.
#]=======================================================================]

arm_config_option(
    NAME TFA_EL3_EXCEPTION_HANDLING
    HELP "Enable handling of exceptions targeted at EL3."
    DEPENDS TFA_ARCH_STATE_AARCH64)

target_compile_definitions(tfa-common
    INTERFACE "EL3_EXCEPTION_HANDLING=$<BOOL:${TFA_EL3_EXCEPTION_HANDLING}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FAULT_INJECTION_SUPPORT

   ARMv8.4-A extensions introduced support for fault injection from lower ELs,
   and this build option enables lower ELs to use Error Records accessed via
   System Registers to inject faults.

   This feature is intended for testing purposes only, and it is advisable to
   keep it disabled for production images.

   Disabled by default. Forcibly disabled unless building for AArch64.
#]=======================================================================]

arm_config_option(
    NAME TFA_FAULT_INJECTION_SUPPORT
    HELP "Enable fault injection support."
    DEPENDS TFA_ARCH VERSION_GREATER_EQUAL 8.4)

target_compile_definitions(tfa-common
    INTERFACE "FAULT_INJECTION_SUPPORT=$<BOOL:${TFA_FAULT_INJECTION_SUPPORT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_HANDLE_EA_EL3_FIRST

   Traps External Aborts and SError Interrupt in EL3 (i.e. in BL31). If
   disabled, these exceptions are instead trapped in the current exception
   level, or in EL1 if the current exception level is EL0.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_HANDLE_EA_EL3_FIRST
    HELP "Trap External Aborts and SError Interrupts in EL3.")

target_compile_definitions(tfa-common
    INTERFACE "HANDLE_EA_EL3_FIRST=$<BOOL:${TFA_HANDLE_EA_EL3_FIRST}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_HW_ASSISTED_COHERENCY

   On most Arm systems to-date, platform-specific software operations are
   required for CPUs to enter and exit coherency. However, newer systems exist
   where CPUs' entry to and exit from coherency is managed in hardware. Such
   systems require software to only initiate these operations, and the rest is
   managed in hardware, minimizing active software management. In such systems,
   this option allows TF-A to carry out build and run-time optimizations during
   boot and power management operations.  is also enabled.

   If this flag is disabled while the platform includes cores that manage
   coherency in hardware, then a compilation error is generated. This is due to
   the fact that a system cannot have, at the same time, cores that manage
   coherency in hardware and cores that don't. In other words, a platform cannot
   have, at the same time, cores that require ``TFA_HW_ASSISTED_COHERENCY=TRUE``
   and cores that require ``TFA_HW_ASSISTED_COHERENCY=FALSE``.

   Disabled by default. Forcibly disabled unless warm boot D-cache early is
   enabled and version 2 of the translation tables library is in use.
#]=======================================================================]

arm_config_option(
    NAME TFA_HW_ASSISTED_COHERENCY HIDDEN
    HELP "Does this platform support hardware-assisted coherency?"
    DEPENDS TFA_WARMBOOT_ENABLE_DCACHE_EARLY)

target_compile_definitions(tfa-common
    INTERFACE "HW_ASSISTED_COHERENCY=$<BOOL:${TFA_HW_ASSISTED_COHERENCY}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_JUNO_AARCH32_EL3_RUNTIME

   Enables execution of EL3 runtime software in AArch32 mode, which is required
   to run AArch32 on Juno. This option builds BL1 and BL2 in AArch64 mode and
   facilitates the loading of ``SP_MIN`` and BL33 as AArch32 executable images.

   Disable by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_JUNO_AARCH32_EL3_RUNTIME
    HELP "Enable execution of EL3 runtime software in AArch32 mode.")

target_compile_definitions(tfa-common
    INTERFACE "JUNO_AARCH32_EL3_RUNTIME=$<BOOL:${TFA_JUNO_AARCH32_EL3_RUNTIME}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_LOG_LEVEL

   Chooses the log level, which controls the amount of console log output
   compiled into the build. This should be one of the following:

   - ``None``: No log output
   - ``Error``: Error log output
   - ``Notice``: Notice log output
   - ``Warning``: Warning log output
   - ``Info``: Informational log output
   - ``Verbose``: Verbose log output

   All log output up to and including the selected log level is compiled into
   the build.

   Defaults to ``Info`` if building in debug mode, otherwise ``Notice``.
#]=======================================================================]

if(TFA_DEBUG)
    arm_config_option_override(
        NAME TFA_LOG_LEVEL
        DEFAULT "Info")
endif()

arm_config_option(
    NAME TFA_LOG_LEVEL
    HELP "Chooses the log level, which controls the amount of console log output compiled into the build."
    STRINGS "None" "Error" "Notice" "Warning" "Info" "Verbose"
    DEFAULT "Notice")

target_compile_definitions(tfa-common
    INTERFACE "LOG_LEVEL=$<UPPER_CASE:LOG_LEVEL_${TFA_LOG_LEVEL}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_MEASURED_BOOT

   Enables support for the Measured Boot feature.

   Disabled by default. Forcibly disabled unless :ref:``TFA_TRUSTED_BOOT`` is
   enabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_MEASURED_BOOT
    HELP "Enable support for Measured Boot."
    DEPENDS TFA_TRUSTED_BOOT)

target_compile_definitions(tfa-common
    INTERFACE "MEASURED_BOOT=$<BOOL:${TFA_MEASURED_BOOT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PROGRAMMABLE_RESET_ADDRESS

   This option indicates whether the reset vector address can be programmed or
   is fixed on the platform. If enabled it is programmable, otherwise it is
   fixed. If the platform has a programmable reset address, it is expected that
   a CPU will start executing code directly at the right address, both on a cold
   and warm reset. In this case, there is no need to identify the entrypoint on
   boot and the boot path can be optimized. The ``plat_get_my_entrypoint()``
   platform porting interface does not need to be implemented in this case.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_PROGRAMMABLE_RESET_ADDRESS
    HELP "Whether the reset vector address can be programmed.")

target_compile_definitions(tfa-common
    INTERFACE "PROGRAMMABLE_RESET_ADDRESS=$<BOOL:${TFA_PROGRAMMABLE_RESET_ADDRESS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PSCI_EXTENDED_STATE_ID

   Per the PSCI 1.0 specification, there are two formats possible for the PSCI
   State-ID format: original and extended. When enabled, this option configures
   the generic PSCI layer to use the extended format.

   This option should be overridden by the platform, and it governs the return
   value of the ``PSCI_FEATURES`` API for the ``CPU_SUSPEND`` SMC function ID.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_PSCI_EXTENDED_STATE_ID HIDDEN
    HELP "Use the extended PSCI State-ID encoding.")

target_compile_definitions(tfa-common
    INTERFACE "PSCI_EXTENDED_STATE_ID=$<BOOL:${TFA_PSCI_EXTENDED_STATE_ID}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RAS_EXTENSION

   Enables RAS features. RAS is an optional extension for Armv8-A CPUs prior to
   Armv8.2-A, at which point it became mandatory.

   Disabled by default. Forcibly disabled if :ref:``TFA_HANDLE_EA_EL3_FIRST`` is
   disabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_RAS_EXTENSION
    HELP "Enable the Armv8-A RAS extension."
    DEPENDS TFA_HANDLE_EA_EL3_FIRST)

target_compile_definitions(tfa-common
    INTERFACE "RAS_EXTENSION=$<BOOL:${TFA_RAS_EXTENSION}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RECLAIM_INIT_CODE

   A significant amount of the code used for the initialization of BL31 is not
   needed again after boot time. In order to reduce the runtime memory
   footprint, the memory used for this code can be reclaimed after
   initialization. This option enables this reclamation.

   Certain boot-time functions are marked with the ``__init`` attribute to
   enable this reclamation.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_RECLAIM_INIT_CODE
    HELP "Enable initialization code reclamation."
    DEPENDS NOT TFA_SEPARATE_NOBITS_REGION)

target_compile_definitions(tfa-common
    INTERFACE "RECLAIM_INIT_CODE=$<BOOL:${TFA_RECLAIM_INIT_CODE}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RESET_TO_BL31

   Use the BL31 entrypoint as the CPU reset vector instead of the BL1
   entrypoint.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_RESET_TO_BL31
    HELP "Reset into BL31 instead of BL1.")

target_compile_definitions(tfa-common
    INTERFACE "RESET_TO_BL31=$<BOOL:${TFA_RESET_TO_BL31}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SDEI_SUPPORT

   Enables support for the Software Delegated Exception Interface in BL31.

   Disabled by default. Forcibly disabled unless
   :ref:``TFA_EL3_EXCEPTION_HANDLING`` is enabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_SDEI_SUPPORT
    HELP "Enables support for the Software Delegated Exception Interface in BL31."
    DEPENDS TFA_EL3_EXCEPTION_HANDLING)

target_compile_definitions(tfa-common
    INTERFACE "SDEI_SUPPORT=$<BOOL:${TFA_SDEI_SUPPORT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SEPARATE_CODE_AND_RODATA

   Whether code and read-only data should be isolated on separate memory pages.
   This is a trade-off between security and memory usage. See "Isolating code
   and read-only data on separate memory pages" section in
   :ref:`Firmware Design`.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_SEPARATE_CODE_AND_RODATA
    HELP "Whether code and read-only data should be isolated on separate memory pages.")

target_compile_definitions(tfa-common
    INTERFACE "SEPARATE_CODE_AND_RODATA=$<BOOL:${TFA_SEPARATE_CODE_AND_RODATA}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SEPARATE_NOBITS_REGION

   Allows the NOBITS sections of BL31 (``.bss``, stacks, page tables, and
   coherent memory) to be allocated in RAM discontiguous from the loaded
   firmware image. When enabled, the platform is expected to provide definitions
   for ``BL31_NOBITS_BASE`` and ``BL31_NOBITS_LIMIT``. When disabled, ``NOBITS``
   sections are placed in RAM immediately following the loaded firmware image.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_SEPARATE_NOBITS_REGION
    HELP "Allows BL31 NOBITS to be independently allocated.")

target_compile_definitions(tfa-common
    INTERFACE "SEPARATE_NOBITS_REGION=$<BOOL:${TFA_SEPARATE_NOBITS_REGION}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPD

   The Secure Payload Dispatcher component to be built into TF-A. Supported
   values are:

     - ``None``: no Secure Payload Dispatcher.

   The SPM Dispatcher cannot be enabled when the
   ``SPM_MM`` option is enabled.

   Defaults to ``None``. Forcibly set to ``None`` if not building for AArch64,
   if :ref:``TFA_EL3_PAYLOAD_BASE`` is enabled, or if ``SPM_MM`` is enabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_SPD
    HELP "Path to the Secure Payload Dispatcher component sources."
    STRINGS "None"
    DEPENDS (NOT TFA_ARCH_STATE_AARCH64) OR (NOT TFA_EL3_PAYLOAD_BASE))

target_compile_definitions(tfa-common
    INTERFACE "SPD_$<LOWER_CASE:$<MAKE_C_IDENTIFIER:${TFA_SPD}>>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPM_MM

   Enable the Management Mode (MM)-based Secure Partition Manager (SPM)
   implementation.

   Disabled by default. Forcibly disabled if using the SPM Dispatcher, or if
   EL3 exceptions are disabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_SPM_MM
    HELP "Enable the Management Mode (MM)-based Secure Partition Manager (SPM) implementation."
    DEPENDS TFA_EL3_EXCEPTION_HANDLING AND (NOT TFA_SPD STREQUAL "SPMD"))

target_compile_definitions(tfa-common
    INTERFACE "SPM_MM=$<BOOL:${TFA_SPM_MM}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TRNG_SUPPORT

   Enables support for the True Random Number Generation interface.
#]=======================================================================]

arm_config_option(
    NAME TFA_TRNG_SUPPORT
    HELP "Enable support for the True Random Number Generation interface.")

target_compile_definitions(tfa-common
    INTERFACE "TRNG_SUPPORT=$<BOOL:${TFA_TRNG_SUPPORT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TRUSTED_BOARD_BOOT

   Includes support for the Trusted Board Boot feature. When enabled, BL1 and
   BL2 images include support for loading and verifying the certificates and
   images in a FIP, and BL1 includes support for Firmware Update. Generation and
   inclusion of certificates in the FIP and FWU FIP depends upon the value of
   the :ref:``TFA_GENERATE_COT`` option.

   Disabled by default. Forcibly disabled unless :ref:``TFA_CREATE_KEYS`` is
   enabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_TRUSTED_BOARD_BOOT
    HELP "Include support for the Trusted Board Boot feature.")

target_compile_definitions(tfa-common
    INTERFACE "TRUSTED_BOARD_BOOT=$<BOOL:${TFA_TRUSTED_BOARD_BOOT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TSP_NS_INTR_ASYNC_PREEMPT

   A non zero value enables the interrupt routing model which routes non-secure interrupts asynchronously from TSP to EL3 causing immediate preemption of TSP. The EL3 is responsible for saving and restoring the TSP context in this routing model. The default routing model (when the value is 0) is to route non-secure interrupts to TSP allowing it to save its context and hand over synchronously to EL3 via an SMC.

   .. note::
      When ``EL3_EXCEPTION_HANDLING`` is ``1``, ``TSP_NS_INTR_ASYNC_PREEMPT`` must also be set to ``1``.
#]=======================================================================]

arm_config_option(
    NAME TFA_TSP_NS_INTR_ASYNC_PREEMPT
    HELP "A non zero value enables the interrupt routing model which routes non-secure interrupts asynchronously from TSP to EL3 causing immediate preemption of TSP. The EL3 is responsible for saving and restoring the TSP context in this routing model. The default routing model (when the value is 0) is to route non-secure interrupts to TSP allowing it to save its context and hand over synchronously to EL3 via an SMC.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_COHERENT_MEM

   This flag determines whether to include the coherent memory region in the BL
   memory map or not (see "Use of Coherent memory in TF-A" section in
   :ref:`Firmware Design`). If enabled, coherent memory region is included,
   otherwise coherent memory region is excluded.

   Enabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_USE_COHERENT_MEM
    HELP "Include the coherent memory region in the BL memory map."
    DEFAULT TRUE
    DEPENDS NOT TFA_HW_ASSISTED_COHERENCY)

target_compile_definitions(tfa-common
    INTERFACE "USE_COHERENT_MEM=$<BOOL:${TFA_USE_COHERENT_MEM}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_DEBUGFS

   When enabled, this option activates a virtual filesystem interface through
   BL31 as a SiP SMC function.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_USE_DEBUGFS
    HELP "Enable DebugFS SiP SMC function.")

target_compile_definitions(tfa-common
    INTERFACE "USE_DEBUGFS=$<BOOL:${TFA_USE_DEBUGFS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_COT_DESC_IN_DTB

   This options determines whether CoT descriptors are crated at runtime using
   the firmware configuration framework. If enabled, CoT descriptors are
   statically captured in the ``tb_fw_config`` file in the form of device tree
   nodes and properties. Currently, CoT descriptors used by BL2 are moved to the
   device tree and CoT descriptors used by BL1 are retained in the code base
   statically.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_COT_DESC_IN_DTB
    HELP "Load CoT descriptors through the firmware configuration framework?")

target_compile_definitions(tfa-common
    INTERFACE "COT_DESC_IN_DTB=$<BOOL:${COT_DESC_IN_DTB}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SDEI_IN_FCONF

   Configures SDEI setup at runtime using the firmware configuration framework.
   The platform-specific SDEI shared and private events configuration is
   retrieved from the device tree rather than static C structures at compile
   time.

   Disabled by default. Forcibly disabled unless :ref:``TFA_SDEI_SUPPORT`` is
   enabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_SDEI_IN_FCONF
    HELP "Configures SDEI setup at runtime using the firmware configuration framework."
    DEPENDS TFA_SDEI_SUPPORT)

target_compile_definitions(tfa-common
    INTERFACE "SDEI_IN_FCONF=$<BOOL:${TFA_SDEI_IN_FCONF}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SEC_INT_DESC_IN_FCONF

   This option determines whether to configure Group 0 and Group 1 secure
   interrupts using the firmware configuration framework. The platform-specific
   secure interrupt property descriptor is retrieved from the device tree at
   runtime rather than depending on static C structures at compile-time.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_SEC_INT_DESC_IN_FCONF
    HELP "Configure secure interrupts using the firmware configuration framework?")

target_compile_definitions(tfa-common
    INTERFACE "SEC_INT_DESC_IN_FCONF=$<BOOL:${SEC_INT_DESC_IN_FCONF}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_ROMLIB

   Enables support for Library at ROM. This feature creates a library of
   functions to be placed in ROM and thus reduces SRAM usage. Refer to
   :ref:`Library at ROM` for further details.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_USE_ROMLIB
    HELP "Enable Library at ROM support.")

target_compile_definitions(tfa-common
    INTERFACE "USE_ROMLIB=$<BOOL:${TFA_USE_ROMLIB}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_WARMBOOT_ENABLE_DCACHE_EARLY

   Enables D-caches early on the CPU after warm boot. This is applicable for
   platforms which do not require interconnect programming to enable cache
   coherency (e.g., single-cluster platforms). If this option is enabled, then
   the warm boot path enables D-caches immediately after enabling the MMU.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_WARMBOOT_ENABLE_DCACHE_EARLY
    HELP "Enable D-caches immediately after enabling the MMU?")

target_compile_definitions(tfa-common
    INTERFACE "WARMBOOT_ENABLE_DCACHE_EARLY=$<BOOL:${TFA_WARMBOOT_ENABLE_DCACHE_EARLY}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RAS_TRAP_LOWER_EL_ERR_ACCESS

   Enables the ``SCR_EL3.TERR`` bit to trap access to the RAS ``ERR`` and RAS
   ``ERX`` registers from lower ELs.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_RAS_TRAP_LOWER_EL_ERR_ACCESS
    HELP "Enables RAS trapping from lower ELs.")

target_compile_definitions(tfa-common
    INTERFACE "RAS_TRAP_LOWER_EL_ERR_ACCESS=$<BOOL:${RAS_TRAP_LOWER_EL_ERR_ACCESS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_SP804_TIMER

   Use the SP804 timer instead of the Generic Timer for functions that wait for
   an arbitrary time length (``udelay`` and ``mdelay``).

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_USE_SP804_TIMER
    HELP "Prefer the SP804 over the generic timer for certain functions?")

target_compile_definitions(tfa-common
    INTERFACE "USE_SP804_TIMER=$<BOOL:${TFA_USE_SP804_TIMER}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_SPINLOCK_CAS

   By default, a load-/store-exclusive instruction pair is used to implement
   spinlocks. When enabled, a spinlock implementation using the ARMv8.1-A LSE
   Compare and Swap instructions is used instead.

   Disabled by default. Forcibly disabled if not building for AArch64.
#]=======================================================================]

arm_config_option(
    NAME TFA_USE_SPINLOCK_CAS
    HELP "Use Armv8.1-A LSE Compare and Swap instructions in spinlocks?"
    DEPENDS TFA_ARCH_STATE_AARCH64)

target_compile_definitions(tfa-common
    INTERFACE "USE_SPINLOCK_CAS=$<BOOL:${TFA_USE_SPINLOCK_CAS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_TBBR_DEFS

   Uses OID definitions provided by TBBR instead of expecting the platform to
   define them.

   Enabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_USE_TBBR_DEFS
    HELP "Use generic TBBR-defined OIDs?"
    DEFAULT TRUE)

target_compile_definitions(tfa-common
    INTERFACE "USE_TBBR_DEFS=$<BOOL:${TFA_USE_TBBR_DEFS}>")
