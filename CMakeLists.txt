#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
Trusted Firmware-A
------------------

This file is the main entry-point of the TF-A CMake build system, or - in CMake
terms - the top-level CMake list file. Its main purpose is to integrate all the
sub-modules into a single build flow, and to set some global settings.
#]=======================================================================]

cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Ext")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/HwPlat")

include(CheckCCompilerFlag)
include(CheckSymbolExists)
include(TFAConfigOption)
include(TFAConfigOptionOverride)

#
# Gather the platform paths that we maintain within the repository. We'll use
# this to get the short names of each platform so that users don't need to
# provide the path every time.
#

list(APPEND valid_platforms "FVP")
list(APPEND valid_platform_source_dirs "arm/board/fvp")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PLATFORM

   The name of the target platform. For out-of-tree platforms this should be
   specified in the ``Platform.cmake`` file in the root directory of your
   platform.
#]=======================================================================]

tfa_config_option(
    NAME TFA_PLATFORM
    HELP "Platform to build."
    STRINGS ${valid_platforms})

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PLATFORM_SOURCE_DIR

   Path to a platform directory for out-of-tree platforms. You should not set
   this for in-tree platforms.
#]=======================================================================]

if(NOT TFA_PLATFORM_SOURCE_DIR)
    list(FIND valid_platforms "${TFA_PLATFORM}" idx)
    list(GET valid_platform_source_dirs "${idx}" TFA_PLATFORM_SOURCE_DIR)
endif()

get_filename_component(TFA_PLATFORM_SOURCE_DIR "${TFA_PLATFORM_SOURCE_DIR}"
                       ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}/plat")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PLATFORM_BINARY_DIR

   Path fragment to use for out-of-tree platforms, representing the subdirectory
   relative to the build directory for compiled platform files.
#]=======================================================================]

if(TFA_PLATFORM_BINARY_DIR)
    file(RELATIVE_PATH TFA_PLATFORM_BINARY_DIR "${CMAKE_BINARY_DIR}"
         "${TFA_PLATFORM_BINARY_DIR}")
else()
    file(RELATIVE_PATH TFA_PLATFORM_BINARY_DIR "${CMAKE_SOURCE_DIR}"
         "${TFA_PLATFORM_SOURCE_DIR}")
endif()

if(TFA_PLATFORM_BINARY_DIR MATCHES "\\.\\.")
    # cmake-format: off
    message(FATAL_ERROR
        "Invalid platform binary directory.\n"

        "Please ensure your platform binary directory is a path relative to "
        "the project build directory (`${CMAKE_BINARY_DIR}`). This path is "
        "used as the binary directory for compiled platform object files.")
    # cmake-format: on
endif()

if(NOT TFA_PLATFORM_BINARY_DIR)
    # cmake-format: off
    message(FATAL_ERROR
        "Missing platform binary directory!\n"

        "Because this is an out-of-tree build, we need to know where to output "
        "compiled files for your platform. This is a path relative to the "
        "build directory (`${CMAKE_BINARY_DIR}`) you have already set, and can "
        "be configured with `TFA_BINARY_DIR`.")
    # cmake-format: on
endif()

#
# Individual platforms are configured based on the `Platform.cmake` metadata
# list file living in the platform source directory that the user provides to
# us, which determines the default set of configuration options, including the
# toolchain file.
#

include("${TFA_PLATFORM_SOURCE_DIR}/Platform.cmake")

if(NOT TFA_PLATFORM)
    # cmake-format: off
    message(FATAL_ERROR
        "Missing platform name!\n"

        "${TFA_PLATFORM_SOURCE_DIR}/Platform.cmake\n"

        "Out-of-tree platforms must provide a platform name. You can do so by "
        "setting `TFA_PLATFORM` in your `Platform.cmake` file.")
    # cmake-format: on
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TOOLCHAINS

    A list of toolchains supported by the platform. These correspond to the
    filenames of the toolchain files in the ``toolchains`` of the platform
    source directory. It is up to the platform to provide this list.

.. cmake:variable:: TFA_TOOLCHAIN

   Name of the toolchain to use. The supported toolchains are provided by the
   platform being configured. If `CMAKE_TOOLCHAIN_FILE`_ is configured, this
   configuration option is disabled.

.. _`CMAKE_TOOLCHAIN_FILE`: https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html
#]=======================================================================]

if(NOT TFA_TOOLCHAINS)
    # cmake-format: off
    message(FATAL_ERROR
        "No toolchains defined!\n"

        "Please ensure your platform has provided a list of supported "
        "toolchains by setting `TFA_TOOLCHAINS`.")
    # cmake-format: on
endif()

tfa_config_option(
    NAME TFA_TOOLCHAIN
    HELP "Toolchain name."
    STRINGS ${TFA_TOOLCHAINS}
    DEPENDS NOT CMAKE_TOOLCHAIN_FILE)

if(TFA_TOOLCHAIN AND (NOT CMAKE_TOOLCHAIN_FILE))
    set(CMAKE_TOOLCHAIN_FILE
        "${TFA_PLATFORM_SOURCE_DIR}/toolchains/${TFA_TOOLCHAIN}.cmake")
endif()

#
# We're done setting up the toolchain, so we can create the project. This will
# do some of the automatic compiler detection, which we need for setting up
# further configuration options.
#

project(tfa VERSION 2.4 LANGUAGES C ASM)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARCH

   Defines the target build architecture for TF-A, either ``aarch64`` or
   ``aarch32``. This value is derived from the pointer width of the platform.

.. cmake:variable:: TFA_AARCH64

   Defines whether the build architecture is AArch64.

.. cmake:variable:: TFA_AARCH32

   Defines whether the build architecture is AArch32.
#]=======================================================================]

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TFA_ARCH "aarch64")
    set(TFA_AARCH64 TRUE)
    set(TFA_AARCH32 FALSE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TFA_ARCH "aarch32")
    set(TFA_AARCH32 TRUE)
    set(TFA_AARCH64 FALSE)
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_ARCH_MAJOR

   The major version of the Arm Architecture targeted when compiling TF-A. Its
   value must be numeric, and is derived from ``CMAKE_SYSTEM_ARCH``, defaulting
   to ``8`` if it could not be derived. This option may be overridden manually
   only if derivation fails.

.. cmake:variable:: TFA_ARM_ARCH_MINOR

   The minor version of the Arm Architecture targeted when compiling TF-A. Its
   value must be numeric, and is derived from ``CMAKE_SYSTEM_ARCH``, defaulting
   to ``0`` if it could not be derived. This option may be overridden manually
   only if derivation fails.
#]=======================================================================]

string(REGEX MATCH "armv([78])(\\.([1-9]))?-a" _ "${CMAKE_SYSTEM_ARCH}")

if(CMAKE_MATCH_COUNT GREATER 0)
    set(TFA_ARM_ARCH_MAJOR "${CMAKE_MATCH_1}")

    if(CMAKE_MATCH_COUNT GREATER 2)
        set(TFA_ARM_ARCH_MINOR "${CMAKE_MATCH_3}")
    else()
        set(TFA_ARM_ARCH_MINOR "0")
    endif()
endif()

if((NOT DEFINED TFA_ARM_ARCH_MAJOR) OR (NOT DEFINED TFA_ARM_ARCH_MAJOR))
    message(FATAL_ERROR
        "Arm architecture version not specified!\n"

        "Please ensure your toolchain file has configured `TFA_ARM_ARCH_MAJOR` "
        "and `TFA_ARM_ARCH_MINOR`.")
endif()

set(TFA_ARM_ARCH "${TFA_ARM_ARCH_MAJOR}.${TFA_ARM_ARCH_MINOR}")

#
# Configure ``CMAKE_BUILD_TYPE`` defaulting to a minimum-size release build. We
# create our own cache variable using :ref:`TFAConfigOption` because it allows
# us to apply the ``STRINGS`` property and configure the default in one go.
#

tfa_config_option(
    NAME CMAKE_BUILD_TYPE
    HELP "Build type."
    STRINGS "Release" "MinSizeRel" "RelWithDebInfo" "Debug"
    DEFAULT "MinSizeRel"
    DEPENDS NOT CMAKE_CONFIGURATION_TYPES
    FORCE NOT CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TFA_DEBUG TRUE)
else()
    set(TFA_DEBUG FALSE)
endif()

#
# Create the common library target. This target pulls together the various
# libraries that are common to virtually every component, and applies
# definitions for global configuration options.
#

add_library(common INTERFACE)

target_compile_definitions(common
    INTERFACE
        "DEBUG=$<BOOL:${TFA_DEBUG}>")

target_include_directories(common INTERFACE "include")
target_link_libraries(common
    INTERFACE
        arch
        common-bl
        common-platform
        driver-multi-console
        lib-c)

target_compile_definitions(common
    INTERFACE
        "ARM_ARCH_MAJOR=${TFA_ARM_ARCH_MAJOR}"
        "ARM_ARCH_MINOR=${TFA_ARM_ARCH_MINOR}"
        "ENABLE_ASSERTIONS=!NDEBUG")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_AARCH32_INSTRUCTION_SET

   Choose the AArch32 instruction set that the compiler should use. Valid values are T32 and A32. It defaults to T32 due to code having a smaller resulting size.
#]=======================================================================]

tfa_config_option(
    NAME TFA_AARCH32_INSTRUCTION_SET
    HELP "Choose the AArch32 instruction set that the compiler should use. Valid values are T32 and A32. It defaults to T32 due to code having a smaller resulting size.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_AARCH32_SP

   Choose the AArch32 Secure Payload component to be built as as the BL32 image when ``ARCH=aarch32``. The value should be the path to the directory containing the SP source, relative to the ``bl32/``; the directory is expected to contain a makefile called ``<aarch32_sp-value>.mk``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_AARCH32_SP
    HELP "Choose the AArch32 Secure Payload component to be built as as the BL32 image when ``ARCH=aarch32``. The value should be the path to the directory containing the SP source, relative to the ``bl32/``; the directory is expected to contain a makefile called ``<aarch32_sp-value>.mk``.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARCH

   Choose the target build architecture for TF-A. It can take either ``aarch64`` or ``aarch32`` as values. By default, it is defined to ``aarch64``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ARCH
    HELP "Choose the target build architecture for TF-A. It can take either ``aarch64`` or ``aarch32`` as values. By default, it is defined to ``aarch64``.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_ARCH_FEATURE

   Optional Arm Architecture build option which specifies one or more feature modifiers. This option has the form ``[no]feature+...`` and defaults to ``none``. It translates into compiler option ``-march=armvX[.Y]-a+[no]feature+...``. See compiler's documentation for the list of supported feature modifiers.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ARM_ARCH_FEATURE
    HELP "Optional Arm Architecture build option which specifies one or more feature modifiers. This option has the form ``[no]feature+...`` and defaults to ``none``. It translates into compiler option ``-march=armvX[.Y]-a+[no]feature+...``. See compiler's documentation for the list of supported feature modifiers.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_ARCH_MAJOR

   The major version of Arm Architecture to target when compiling TF-A. Its value must be numeric, and defaults to 8 . See also, *Armv8 Architecture Extensions* and *Armv7 Architecture Extensions* in :ref:`Firmware Design`.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ARM_ARCH_MAJOR
    HELP "The major version of Arm Architecture to target when compiling TF-A. Its value must be numeric, and defaults to 8 . See also, *Armv8 Architecture Extensions* and *Armv7 Architecture Extensions* in :ref:`Firmware Design`.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_ARCH_MINOR

   The minor version of Arm Architecture to target when compiling TF-A. Its value must be a numeric, and defaults to 0. See also, *Armv8 Architecture Extensions* in :ref:`Firmware Design`.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ARM_ARCH_MINOR
    HELP "The minor version of Arm Architecture to target when compiling TF-A. Its value must be a numeric, and defaults to 0. See also, *Armv8 Architecture Extensions* in :ref:`Firmware Design`.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL2

   This is an optional build option which specifies the path to BL2 image for the ``fip`` target. In this case, the BL2 in the TF-A will not be built.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL2
    HELP "This is an optional build option which specifies the path to BL2 image for the ``fip`` target. In this case, the BL2 in the TF-A will not be built.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL2U

   This is an optional build option which specifies the path to BL2U image. In this case, the BL2U in TF-A will not be built.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL2U
    HELP "This is an optional build option which specifies the path to BL2U image. In this case, the BL2U in TF-A will not be built.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL2_AT_EL3

   Enables the use of BL2 at the EL3 execution level.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL2_AT_EL3
    HELP "Enable the use of BL2 at EL3 execution level.")

target_compile_definitions(common
    INTERFACE
        "BL2_AT_EL3=$<BOOL:${TFA_BL2_AT_EL3}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL2_IN_XIP_MEM

   In some situations BL2 will be stored in eXecute In Place (XIP) memory, like
   BL1. In these situations, it is necessary to initialize the RW sections in
   RAM, while leaving the RO sections in place, which this option facilitates.

   Disabled by default. Forcibly disabled if BL2 is not configured to execute
   from EL3.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL2_IN_XIP_MEM
    HELP "Execute BL2 from eXecute In Place (XIP) memory."
    DEPENDS TFA_BL2_AT_EL3)

target_compile_definitions(common
    INTERFACE
        "BL2_IN_XIP_MEM=$<BOOL:${TFA_BL2_IN_XIP_MEM}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL31

   This is an optional build option which specifies the path to BL31 image for the ``fip`` target. In this case, the BL31 in TF-A will not be built.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL31
    HELP "This is an optional build option which specifies the path to BL31 image for the ``fip`` target. In this case, the BL31 in TF-A will not be built.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL31_KEY

   This option is used when ``GENERATE_COT=1``. It specifies the file that contains the BL31 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL31_KEY
    HELP "This option is used when ``GENERATE_COT=1``. It specifies the file that contains the BL31 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL32

   This is an optional build option which specifies the path to BL32 image for the ``fip`` target. In this case, the BL32 in TF-A will not be built.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL32
    HELP "This is an optional build option which specifies the path to BL32 image for the ``fip`` target. In this case, the BL32 in TF-A will not be built.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL32_EXTRA1

   This is an optional build option which specifies the path to Trusted OS Extra1 image for the  ``fip`` target.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL32_EXTRA1
    HELP "This is an optional build option which specifies the path to Trusted OS Extra1 image for the  ``fip`` target.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL32_EXTRA2

   This is an optional build option which specifies the path to Trusted OS Extra2 image for the ``fip`` target.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL32_EXTRA2
    HELP "This is an optional build option which specifies the path to Trusted OS Extra2 image for the ``fip`` target.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL32_KEY

   This option is used when ``GENERATE_COT=1``. It specifies the file that contains the BL32 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL32_KEY
    HELP "This option is used when ``GENERATE_COT=1``. It specifies the file that contains the BL32 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL33

   Path to BL33 image in the host file system. This is mandatory for ``fip`` target in case TF-A BL2 is used.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL33
    HELP "Path to BL33 image in the host file system. This is mandatory for ``fip`` target in case TF-A BL2 is used.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BL33_KEY

   This option is used when ``GENERATE_COT=1``. It specifies the file that contains the BL33 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BL33_KEY
    HELP "This option is used when ``GENERATE_COT=1``. It specifies the file that contains the BL33 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_POINTER_AUTHENTICATION

   Enables pointer authentication when compiling for Armv8.3-A or greater. This
   option supports the following:

   - ``None``: disables pointer authentication
   - ``Standard``: applies pointer authentication to all vulnerable functions
   - ``Extensive``: applies pointer authentication to all functions

   Defaults to ``None``. Forcibly disabled if the compiler or target
   architecture do not support pointer authentication.
#]=======================================================================]

check_c_compiler_flag("-mbranch-protection=pac-ret" have_gnu_pac_ret)
check_c_compiler_flag("-mbranch-protection=pac-ret+leaf" have_gnu_pac_ret_leaf)

set(pauth_strings "None")

if(have_gnu_pac_ret)
    list(APPEND pauth_strings "Standard")
    list(APPEND pauth_gnu_Standard "pac-ret")

    if(have_gnu_pac_ret_leaf)
        list(APPEND pauth_strings "Extensive")
        list(APPEND pauth_gnu_Extensive "pac-ret")
        list(APPEND pauth_gnu_Extensive "leaf")
    endif()
endif()

tfa_config_option(
    NAME TFA_POINTER_AUTHENTICATION
    HELP "Enables pointer authentication."
    STRINGS ${pauth_strings}
    DEPENDS TFA_ARM_ARCH VERSION_GREATER_EQUAL 8.0)

list(APPEND mbranch_protection ${pauth_gnu_${TFA_POINTER_AUTHENTICATION}})

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_PAUTH

   Determines whether pointer authentication is enabled. This variable is a
   shorthand method of checking if :ref:``TFA_POINTER_AUTHENTICATION`` is not
   ``None``.
#]=======================================================================]

if(NOT TFA_POINTER_AUTHENTICATION STREQUAL "None")
    tfa_config_option_override(
        NAME TFA_ENABLE_PAUTH
        FORCE TRUE)
endif()

tfa_config_option(
    NAME TFA_ENABLE_PAUTH
    HELP "Whether pointer authentication is enabled."
    FORCE)

target_compile_definitions(common
    INTERFACE
        "ENABLE_PAUTH=$<BOOL:${TFA_ENABLE_PAUTH}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BRANCH_TARGET_IDENTIFICATION

   Enables branch target identification when compiling for Armv8.5-A or greater.

   Disabled by default. Forcibly disabled if the compiler or target architecture
   do not support branch target identification.
#]=======================================================================]

check_c_compiler_flag("-mbranch-protection=bti" have_gnu_bti)

tfa_config_option(
    NAME TFA_BRANCH_TARGET_IDENTIFICATION
    HELP "Enables branch target identification."
    DEPENDS have_gnu_bti AND (TFA_ARM_ARCH VERSION_GREATER_EQUAL 8.0))

if(TFA_BRANCH_TARGET_IDENTIFICATION)
    list(APPEND mbranch_protection "bti")
endif()

if(mbranch_protection)
    target_compile_options(common
        INTERFACE
            "-mbranch-protection=$<JOIN:${mbranch_protection},+>")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BUILD_MESSAGE_TIMESTAMP

   String used to identify the time and date of the compilation of each build. It must be set to a C string (including quotes where applicable). Defaults to a string that contains the time and date of the compilation.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BUILD_MESSAGE_TIMESTAMP
    HELP "String used to identify the time and date of the compilation of each build. It must be set to a C string (including quotes where applicable). Defaults to a string that contains the time and date of the compilation.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BUILD_STRING

   Input string for VERSION_STRING, which allows the TF-A build to be uniquely identified. Defaults to the current git commit id.
#]=======================================================================]

tfa_config_option(
    NAME TFA_BUILD_STRING
    HELP "Input string for VERSION_STRING, which allows the TF-A build to be uniquely identified. Defaults to the current git commit id.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_BUILD_BASE

   Output directory for the build. Defaults to ``./build``
#]=======================================================================]

tfa_config_option(
    NAME TFA_BUILD_BASE
    HELP "Output directory for the build. Defaults to ``./build``")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CFLAGS

   Extra user options appended on the compiler's command line in addition to the options set by the build system.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CFLAGS
    HELP "Extra user options appended on the compiler's command line in addition to the options set by the build system.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_COLD_BOOT_SINGLE_CPU

   This option indicates whether the platform may release several CPUs out of
   reset. If enabled, only one CPU will ever be brought up during cold reset,
   otherwise multiple CPUs may be brought up. If the platform always brings up a
   single CPU, there is no need to distinguish between primary and secondary
   CPUs and the boot path can be optimised. The ``plat_is_my_cpu_primary()`` and
   ``plat_secondary_cold_boot_setup()`` platform porting interfaces do not need
   to be implemented in this case.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_COLD_BOOT_SINGLE_CPU
    HELP "Whether the platform releases only a single CPU out of reset.")

target_compile_definitions(common
    INTERFACE
        "COLD_BOOT_SINGLE_CPU=$<BOOL:${TFA_COLD_BOOT_SINGLE_CPU}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_COT

   When Trusted Boot is enabled, selects the desired chain of trust. Defaults to ``tbbr``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_COT
    HELP "When Trusted Boot is enabled, selects the desired chain of trust. Defaults to ``tbbr``.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CRASH_REPORTING

   Enables a console dump of processor register state when an unexpected
   exception occurs during execution of BL31.

   When building in debug mode, this option is enabled by default. Otherwise,
   this option is disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CRASH_REPORTING
    HELP "Dump processor registers state when an unexpected exception occurs."
    DEFAULT ${TFA_DEBUG})

target_compile_definitions(common
    INTERFACE
        "CRASH_REPORTING=$<BOOL:${TFA_CRASH_REPORTING}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CREATE_KEYS

   This option is used when ``GENERATE_COT=1``. It tells the certificate generation tool to create new keys in case no valid keys are present or specified. Allowed options are '0' or '1'. Default is '1'.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CREATE_KEYS
    HELP "This option is used when ``GENERATE_COT=1``. It tells the certificate generation tool to create new keys in case no valid keys are present or specified. Allowed options are '0' or '1'. Default is '1'.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_AARCH32_REGS

   Boolean option that, when set to 1, will cause the AArch32 system registers to be included when saving and restoring the CPU context. The option must be set to 0 for AArch64-only platforms (that is on hardware that does not implement AArch32, or at least not at EL1 and higher ELs). Default value is 1.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CTX_INCLUDE_AARCH32_REGS
    HELP "Boolean option that, when set to 1, will cause the AArch32 system registers to be included when saving and restoring the CPU context. The option must be set to 0 for AArch64-only platforms (that is on hardware that does not implement AArch32, or at least not at EL1 and higher ELs). Default value is 1.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_EL2_REGS

   This boolean option provides context save/restore operations when entering/exiting an EL2 execution context. This is of primary interest when Armv8.4-SecEL2 extension is implemented. Default is 0 (disabled). This option must be equal to 1 (enabled) when ``SPD=spmd`` and ``SPMD_SPM_AT_SEL2`` is set.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CTX_INCLUDE_EL2_REGS
    HELP "This boolean option provides context save/restore operations when entering/exiting an EL2 execution context. This is of primary interest when Armv8.4-SecEL2 extension is implemented. Default is 0 (disabled). This option must be equal to 1 (enabled) when ``SPD=spmd`` and ``SPMD_SPM_AT_SEL2`` is set.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_FPREGS

   Boolean option that, when set to 1, will cause the FP registers to be included when saving and restoring the CPU context. Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CTX_INCLUDE_FPREGS
    HELP "Boolean option that, when set to 1, will cause the FP registers to be included when saving and restoring the CPU context. Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_NEVE_REGS

   Boolean option that, when set to 1, will cause the Armv8.4-NV registers to be saved/restored when entering/exiting an EL2 execution context. Default value is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CTX_INCLUDE_NEVE_REGS
    HELP "Boolean option that, when set to 1, will cause the Armv8.4-NV registers to be saved/restored when entering/exiting an EL2 execution context. Default value is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_CTX_INCLUDE_PAUTH_REGS

   Boolean option that, when set to 1, enables Pointer Authentication for Secure world. This will cause the ARMv8.3-PAuth registers to be included when saving and restoring the CPU context as part of world switch. Default value is 0 and this is an experimental feature. Note that Pointer Authentication is enabled for Non-secure world irrespective of the value of this flag if the CPU supports it.
#]=======================================================================]

tfa_config_option(
    NAME TFA_CTX_INCLUDE_PAUTH_REGS
    HELP "Boolean option that, when set to 1, enables Pointer Authentication for Secure world. This will cause the ARMv8.3-PAuth registers to be included when saving and restoring the CPU context as part of world switch. Default value is 0 and this is an experimental feature. Note that Pointer Authentication is enabled for Non-secure world irrespective of the value of this flag if the CPU supports it.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_DECRYPTION_SUPPORT

   This build flag enables the user to select the authenticated decryption algorithm to be used to decrypt firmware/s during boot. It accepts 2 values: ``aes_gcm`` and ``none``. The default value of this flag is ``none`` to disable firmware decryption which is an optional feature as per TBBR. Also, it is an experimental feature.
#]=======================================================================]

tfa_config_option(
    NAME TFA_DECRYPTION_SUPPORT
    HELP "This build flag enables the user to select the authenticated decryption algorithm to be used to decrypt firmware/s during boot. It accepts 2 values: ``aes_gcm`` and ``none``. The default value of this flag is ``none`` to disable firmware decryption which is an optional feature as per TBBR. Also, it is an experimental feature.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_DISABLE_BIN_GENERATION

   Boolean option to disable the generation of the binary image. If set to 1, then only the ELF image is built. 0 is the default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_DISABLE_BIN_GENERATION
    HELP "Boolean option to disable the generation of the binary image. If set to 1, then only the ELF image is built. 0 is the default.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_DISABLE_MTPMU

   Boolean option to disable FEAT_MTPMU if implemented (Armv8.6 onwards). Its default value is 0 to keep consistency with platforms that do not implement FEAT_MTPMU. For more information on FEAT_MTPMU, check the latest Arm ARM.
#]=======================================================================]

tfa_config_option(
    NAME TFA_DISABLE_MTPMU
    HELP "Boolean option to disable FEAT_MTPMU if implemented (Armv8.6 onwards). Its default value is 0 to keep consistency with platforms that do not implement FEAT_MTPMU. For more information on FEAT_MTPMU, check the latest Arm ARM.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_DYN_DISABLE_AUTH

   Provides the capability to dynamically disable Trusted Board Boot authentication at runtime. This option is meant to be enabled only for development platforms. ``TRUSTED_BOARD_BOOT`` flag must be set if this flag has to be enabled. 0 is the default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_DYN_DISABLE_AUTH
    HELP "Provides the capability to dynamically disable Trusted Board Boot authentication at runtime. This option is meant to be enabled only for development platforms. ``TRUSTED_BOARD_BOOT`` flag must be set if this flag has to be enabled. 0 is the default.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_E

   Boolean option to make warnings into errors. Default is 1.
#]=======================================================================]

tfa_config_option(
    NAME TFA_E
    HELP "Boolean option to make warnings into errors. Default is 1.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_EL3_PAYLOAD_BASE

   This option enables booting an EL3 payload instead of the normal boot flow. It must specify the entry point address of the EL3 payload. Please refer to the "Booting an EL3 payload" section for more details.
#]=======================================================================]

tfa_config_option(
    NAME TFA_EL3_PAYLOAD_BASE
    HELP "This option enables booting an EL3 payload instead of the normal boot flow. It must specify the entry point address of the EL3 payload. Please refer to the \"Booting an EL3 payload\" section for more details.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_AMU

   Boolean option to enable Activity Monitor Unit extensions. This is an optional architectural feature available on v8.4 onwards. Some v8.2 implementations also implement an AMU and this option can be used to enable this feature on those systems as well. Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_AMU
    HELP "Boolean option to enable Activity Monitor Unit extensions. This is an optional architectural feature available on v8.4 onwards. Some v8.2 implementations also implement an AMU and this option can be used to enable this feature on those systems as well. Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_ASSERTIONS

   This option controls whether or not calls to ``assert()`` are compiled out. For debug builds, this option defaults to 1, and calls to ``assert()`` are left in place. For release builds, this option defaults to 0 and calls to ``assert()`` function are compiled out. This option can be set independently of ``DEBUG``. It can also be used to hide any auxiliary code that is only required for the assertion and does not fit in the assertion itself.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_ASSERTIONS
    HELP "This option controls whether or not calls to ``assert()`` are compiled out. For debug builds, this option defaults to 1, and calls to ``assert()`` are left in place. For release builds, this option defaults to 0 and calls to ``assert()`` function are compiled out. This option can be set independently of ``DEBUG``. It can also be used to hide any auxiliary code that is only required for the assertion and does not fit in the assertion itself.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_BACKTRACE

   Enables backtraced crash dumps. This is supported for both AArch64 and
   AArch32, but in AArch32 the format of the frame records is not defined in the
   AAPCS and is instead defined by the implementation. This implementation of
   backtracing only supports the format used by GCC when T32 interworking is
   disabled. For this reason, enabling this option in AArch32 will force the
   compiler to only generate A32 code.

   Enabled by default unless building for AArch32.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_BACKTRACE
    HELP "Enable backtracing in crash dumps."
    DEFAULT ${TFA_AARCH32})

if(TFA_ENABLE_BACKTRACE)
    target_link_libraries(common
        INTERFACE
            backtrace)
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_LTO

   Boolean option to enable Link Time Optimization (LTO) support in GCC for TF-A. This option is currently only supported for AArch64. Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_LTO
    HELP "Boolean option to enable Link Time Optimization (LTO) support in GCC for TF-A. This option is currently only supported for AArch64. Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_MPAM_FOR_LOWER_ELS

   Boolean option to enable lower ELs to use MPAM feature. MPAM is an optional Armv8.4 extension that enables various memory system components and resources to define partitions; software running at various ELs can assign themselves to desired partition to control their performance aspects.

   When this option is set to ``1``, EL3 allows lower ELs to access their own MPAM registers without trapping into EL3. This option doesn't make use of partitioning in EL3, however. Platform initialisation code should configure and use partitions in EL3 as required. This option defaults to ``0``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_MPAM_FOR_LOWER_ELS
    HELP "Boolean option to enable lower ELs to use MPAM feature. MPAM is an optional Armv8.4 extension that enables various memory system components and resources to define partitions; software running at various ELs can assign themselves to desired partition to control their performance aspects.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_PIE

   Boolean option to enable Position Independent Executable(PIE) support within generic code in TF-A. This option is currently only supported in BL2_AT_EL3, BL31, and BL32 (TSP). Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_PIE
    HELP "Boolean option to enable Position Independent Executable(PIE) support within generic code in TF-A. This option is currently only supported in BL2_AT_EL3, BL31, and BL32 (TSP). Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_PMF

   Boolean option to enable support for optional Performance Measurement Framework(PMF). Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_PMF
    HELP "Boolean option to enable support for optional Performance Measurement Framework(PMF). Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_PSCI_STAT

   Enables support for the optional PSCI functions ``PSCI_STAT_RESIDENCY`` and
   ``PSCI_STAT_COUNT``. In the absence of an alternative statistics collection
   backend, :ref:``TFA_ENABLE_PMF`` must be enabled. If :ref:``TFA_ENABLE_PMF``
   is enabled, the residency statistics are tracked in software.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_PSCI_STAT
    HELP "Enable support for PSCI statistics tracking.")

target_compile_definitions(common
    INTERFACE
        "ENABLE_PSCI_STAT=$<BOOL:${TFA_ENABLE_PSCI_STAT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_RUNTIME_INSTRUMENTATION

   Enables runtime instrumentation, which injects timestamp collection points
   into TF-A to allow runtime performance to be measured. Currently, only PSCI
   is instrumented.

   Disabled by default. Forcibly disabled unless the Performance Monitoring
   Framework (PMF) is enabled.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_RUNTIME_INSTRUMENTATION
    HELP "Enable runtime instrumentation."
    DEPENDS TFA_ENABLE_PMF)

target_compile_definitions(common
    INTERFACE
        "ENABLE_RUNTIME_INSTRUMENTATION=$<BOOL:${TFA_ENABLE_RUNTIME_INSTRUMENTATION}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_SPE_FOR_LOWER_ELS

   Boolean option to enable Statistical Profiling extensions. This is an optional architectural feature for AArch64. The default is 1 but is automatically disabled when the target architecture is AArch32.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_SPE_FOR_LOWER_ELS
    HELP "Boolean option to enable Statistical Profiling extensions. This is an optional architectural feature for AArch64. The default is 1 but is automatically disabled when the target architecture is AArch32.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_SVE_FOR_NS

   Boolean option to enable Scalable Vector Extension (SVE) for the Non-secure world only. SVE is an optional architectural feature for AArch64. Note that when SVE is enabled for the Non-secure world, access to SIMD and floating-point functionality from the Secure world is disabled. This is to avoid corruption of the Non-secure world data in the Z-registers which are aliased by the SIMD and FP registers. The build option is not compatible with the ``CTX_INCLUDE_FPREGS`` build option, and will raise an assert on platforms where SVE is implemented and ``ENABLE_SVE_FOR_NS`` set to 1. The default is 1 but is automatically disabled when the target architecture is AArch32.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_SVE_FOR_NS
    HELP "Boolean option to enable Scalable Vector Extension (SVE) for the Non-secure world only. SVE is an optional architectural feature for AArch64. Note that when SVE is enabled for the Non-secure world, access to SIMD and floating-point functionality from the Secure world is disabled. This is to avoid corruption of the Non-secure world data in the Z-registers which are aliased by the SIMD and FP registers. The build option is not compatible with the ``CTX_INCLUDE_FPREGS`` build option, and will raise an assert on platforms where SVE is implemented and ``ENABLE_SVE_FOR_NS`` set to 1. The default is 1 but is automatically disabled when the target architecture is AArch32.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENABLE_STACK_PROTECTOR

   Additional stack protection flags, which are passed directly to the compiler.

   If stack protection flags are provided, the platform must implemented the
   ``plat_get_stack_protector_canary()`` function.

   Defaults to nothing.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENABLE_STACK_PROTECTOR
    HELP "Stack protection compiler flags."
    TYPE STRING)

if(TFA_ENABLE_STACK_PROTECTOR)
    target_compile_options(common
        INTERFACE
            "${TFA_ENABLE_STACK_PROTECTOR}")
endif()

target_compile_definitions(common
    INTERFACE
        "STACK_PROTECTOR_ENABLED=$<BOOL:${TFA_ENABLE_STACK_PROTECTOR}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENCRYPT_BL31

   Binary flag to enable encryption of BL31 firmware. This flag depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENCRYPT_BL31
    HELP "Binary flag to enable encryption of BL31 firmware. This flag depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENCRYPT_BL32

   Binary flag to enable encryption of Secure BL32 payload. This flag depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENCRYPT_BL32
    HELP "Binary flag to enable encryption of Secure BL32 payload. This flag depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENC_KEY

   A 32-byte (256-bit) symmetric key in hex string format. It could either be SSK or BSSK depending on ``FW_ENC_STATUS`` flag. This value depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENC_KEY
    HELP "A 32-byte (256-bit) symmetric key in hex string format. It could either be SSK or BSSK depending on ``FW_ENC_STATUS`` flag. This value depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ENC_NONCE

   A 12-byte (96-bit) encryption nonce or Initialization Vector (IV) in hex string format. This value depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ENC_NONCE
    HELP "A 12-byte (96-bit) encryption nonce or Initialization Vector (IV) in hex string format. This value depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERROR_DEPRECATED

   This option decides whether to treat the usage of deprecated platform APIs, helper functions or drivers within Trusted Firmware as error. It can take the value 1 (flag the use of deprecated APIs as error) or 0. The default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ERROR_DEPRECATED
    HELP "This option decides whether to treat the usage of deprecated platform APIs, helper functions or drivers within Trusted Firmware as error. It can take the value 1 (flag the use of deprecated APIs as error) or 0. The default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_EL3_EXCEPTION_HANDLING

   Enables handling of exceptions targeted at EL3. When disabled, no exceptions
   are expected or handled at EL3, and a panic will result.

   Disabled by default. Forcibly disabled if not building for AArch64.
#]=======================================================================]

tfa_config_option(
    NAME TFA_EL3_EXCEPTION_HANDLING
    HELP "Enable handling of exceptions targeted at EL3."
    DEPENDS TFA_AARCH64)

target_compile_definitions(common
    INTERFACE
        "EL3_EXCEPTION_HANDLING=$<BOOL:${TFA_EL3_EXCEPTION_HANDLING}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_EVENT_LOG_LEVEL

   Chooses the log level to use for Measured Boot when ``MEASURED_BOOT`` is enabled. For a list of valid values, see ``LOG_LEVEL``. Default value is 40 (LOG_LEVEL_INFO).
#]=======================================================================]

tfa_config_option(
    NAME TFA_EVENT_LOG_LEVEL
    HELP "Chooses the log level to use for Measured Boot when ``MEASURED_BOOT`` is enabled. For a list of valid values, see ``LOG_LEVEL``. Default value is 40 (LOG_LEVEL_INFO).")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FAULT_INJECTION_SUPPORT

   ARMv8.4-A extensions introduced support for fault injection from lower ELs,
   and this build option enables lower ELs to use Error Records accessed via
   System Registers to inject faults.

   This feature is intended for testing purposes only, and it is advisable to
   keep it disabled for production images.

   Disabled by default. Forcibly disabled unless building for AArch64.
#]=======================================================================]

tfa_config_option(
    NAME TFA_FAULT_INJECTION_SUPPORT
    HELP "Enable fault injection support."
    DEPENDS TFA_ARM_ARCH VERSION_GREATER_EQUAL 8.4)

target_compile_definitions(common
    INTERFACE
        "FAULT_INJECTION_SUPPORT=$<BOOL:${TFA_FAULT_INJECTION_SUPPORT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FIP_NAME

   This is an optional build option which specifies the FIP filename for the ``fip`` target. Default is ``fip.bin``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_FIP_NAME
    HELP "This is an optional build option which specifies the FIP filename for the ``fip`` target. Default is ``fip.bin``.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FWU_FIP_NAME

   This is an optional build option which specifies the FWU FIP filename for the ``fwu_fip`` target. Default is ``fwu_fip.bin``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_FWU_FIP_NAME
    HELP "This is an optional build option which specifies the FWU FIP filename for the ``fwu_fip`` target. Default is ``fwu_fip.bin``.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FW_ENC_STATUS

   Top level firmware's encryption numeric flag, values:

   ::

     0: Encryption is done with Secret Symmetric Key (SSK) which is common for a class of devices.
     1: Encryption is done with Binding Secret Symmetric Key (BSSK) which is unique per device.

   This flag depends on ``DECRYPTION_SUPPORT`` build flag which is marked as experimental.
#]=======================================================================]

tfa_config_option(
    NAME TFA_FW_ENC_STATUS
    HELP "Top level firmware's encryption numeric flag, values:")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GENERATE_COT

   Boolean flag used to build and execute the ``cert_create`` tool to create certificates as per the Chain of Trust described in :ref:`Trusted Board Boot`. The build system then calls ``fiptool`` to include the certificates in the FIP and FWU_FIP. Default value is '0'.

   Specify both ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=1`` to include support for the Trusted Board Boot feature in the BL1 and BL2 images, to generate the corresponding certificates, and to include those certificates in the FIP and FWU_FIP.

   Note that if ``TRUSTED_BOARD_BOOT=0`` and ``GENERATE_COT=1``, the BL1 and BL2 images will not include support for Trusted Board Boot. The FIP will still include the corresponding certificates. This FIP can be used to verify the Chain of Trust on the host machine through other mechanisms.

   Note that if ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=0``, the BL1 and BL2 images will include support for Trusted Board Boot, but the FIP and FWU_FIP will not include the corresponding certificates, causing a boot failure.
#]=======================================================================]

tfa_config_option(
    NAME TFA_GENERATE_COT
    HELP "Boolean flag used to build and execute the ``cert_create`` tool to create certificates as per the Chain of Trust described in :ref:`Trusted Board Boot`. The build system then calls ``fiptool`` to include the certificates in the FIP and FWU_FIP. Default value is '0'.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV2_G0_FOR_EL3

   Unlike GICv3, the GICv2 architecture doesn't have inherent support for specific EL3 type interrupts. Setting this build option to ``1`` assumes GICv2 *Group 0* interrupts are expected to target EL3, both by :ref:`platform abstraction layer<platform Interrupt Controller API>` and :ref:`Interrupt Management Framework<Interrupt Management Framework>`. This allows GICv2 platforms to enable features requiring EL3 interrupt type. This also means that all GICv2 Group 0 interrupts are delivered to EL3, and the Secure Payload interrupts needs to be synchronously handed over to Secure EL1 for handling. The default value of this option is ``0``, which means the Group 0 interrupts are assumed to be handled by Secure EL1.
#]=======================================================================]

tfa_config_option(
    NAME TFA_GICV2_G0_FOR_EL3
    HELP "Unlike GICv3, the GICv2 architecture doesn't have inherent support for specific EL3 type interrupts. Setting this build option to ``1`` assumes GICv2 *Group 0* interrupts are expected to target EL3, both by :ref:`platform abstraction layer<platform Interrupt Controller API>` and :ref:`Interrupt Management Framework<Interrupt Management Framework>`. This allows GICv2 platforms to enable features requiring EL3 interrupt type. This also means that all GICv2 Group 0 interrupts are delivered to EL3, and the Secure Payload interrupts needs to be synchronously handed over to Secure EL1 for handling. The default value of this option is ``0``, which means the Group 0 interrupts are assumed to be handled by Secure EL1.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_HANDLE_EA_EL3_FIRST

   Traps External Aborts and SError Interrupt in EL3 (i.e. in BL31). If
   disabled, these exceptions are instead trapped in the current exception
   level, or in EL1 if the current exception level is EL0.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_HANDLE_EA_EL3_FIRST
    HELP "Trap External Aborts and SError Interrupts in EL3.")

target_compile_definitions(common
    INTERFACE
        "HANDLE_EA_EL3_FIRST=$<BOOL:${TFA_HANDLE_EA_EL3_FIRST}>")

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

tfa_config_option(
    NAME TFA_HW_ASSISTED_COHERENCY
    HELP "On most Arm systems to-date, platform-specific software operations are required for CPUs to enter and exit coherency. However, newer systems exist where CPUs' entry to and exit from coherency is managed in hardware. Such systems require software to only initiate these operations, and the rest is managed in hardware, minimizing active software management. In such systems, this boolean option enables TF-A to carry out build and run-time optimizations during boot and power management operations. This option defaults to 0 and if it is enabled, then it implies ``WARMBOOT_ENABLE_DCACHE_EARLY`` is also enabled."
    DEPENDS TFA_WARMBOOT_ENABLE_DCACHE_EARLY)

target_compile_definitions(common
    INTERFACE
        "HW_ASSISTED_COHERENCY=$<BOOL:${TFA_HW_ASSISTED_COHERENCY}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_INVERTED_MEMMAP

   memmap tool print by default lower addresses at the bottom, higher addresses at the top. This build flag can be set to '1' to invert this behavior. Lower addresses will be printed at the top and higher addresses at the bottom.
#]=======================================================================]

tfa_config_option(
    NAME TFA_INVERTED_MEMMAP
    HELP "memmap tool print by default lower addresses at the bottom, higher addresses at the top. This build flag can be set to '1' to invert this behavior. Lower addresses will be printed at the top and higher addresses at the bottom.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_JUNO_AARCH32_EL3_RUNTIME

   Enables execution of EL3 runtime software in AArch32 mode, which is required
   to run AArch32 on Juno. This option builds BL1 and BL2 in AArch64 mode and
   facilitates the loading of ``SP_MIN`` and BL33 as AArch32 executable images.

   Disable by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_JUNO_AARCH32_EL3_RUNTIME
    HELP "Enable execution of EL3 runtime software in AArch32 mode.")

target_compile_definitions(common
    INTERFACE
        "JUNO_AARCH32_EL3_RUNTIME=$<BOOL:${TFA_JUNO_AARCH32_EL3_RUNTIME}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_KEY_ALG

   This build flag enables the user to select the algorithm to be used for generating the PKCS keys and subsequent signing of the certificate. It accepts 3 values: ``rsa``, ``rsa_1_5`` and ``ecdsa``. The option ``rsa_1_5`` is the legacy PKCS#1 RSA 1.5 algorithm which is not TBBR compliant and is retained only for compatibility. The default value of this flag is ``rsa`` which is the TBBR compliant PKCS#1 RSA 2.1 scheme.
#]=======================================================================]

tfa_config_option(
    NAME TFA_KEY_ALG
    HELP "This build flag enables the user to select the algorithm to be used for generating the PKCS keys and subsequent signing of the certificate. It accepts 3 values: ``rsa``, ``rsa_1_5`` and ``ecdsa``. The option ``rsa_1_5`` is the legacy PKCS#1 RSA 1.5 algorithm which is not TBBR compliant and is retained only for compatibility. The default value of this flag is ``rsa`` which is the TBBR compliant PKCS#1 RSA 2.1 scheme.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_KEY_SIZE

   This build flag enables the user to select the key size for the algorithm specified by ``KEY_ALG``. The valid values for ``KEY_SIZE`` depend on the chosen algorithm and the cryptographic module.

   +-----------+------------------------------------+
   |  KEY_ALG  |        Possible key sizes          |
   +===========+====================================+
   |    rsa    | 1024 , 2048 (default), 3072, 4096* |
   +-----------+------------------------------------+
   |   ecdsa   |            unavailable             |
   +-----------+------------------------------------+

   * Only 2048 bits size is available with CryptoCell 712 SBROM release 1.
     Only 3072 bits size is available with CryptoCell 712 SBROM release 2.
#]=======================================================================]

tfa_config_option(
    NAME TFA_KEY_SIZE
    HELP "This build flag enables the user to select the key size for the algorithm specified by ``KEY_ALG``. The valid values for ``KEY_SIZE`` depend on the chosen algorithm and the cryptographic module.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_HASH_ALG

   This build flag enables the user to select the secure hash algorithm. It accepts 3 values: ``sha256``, ``sha384`` and ``sha512``. The default value of this flag is ``sha256``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_HASH_ALG
    HELP "This build flag enables the user to select the secure hash algorithm. It accepts 3 values: ``sha256``, ``sha384`` and ``sha512``. The default value of this flag is ``sha256``.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_LDFLAGS

   Extra user options appended to the linkers' command line in addition to the one set by the build system.
#]=======================================================================]

tfa_config_option(
    NAME TFA_LDFLAGS
    HELP "Extra user options appended to the linkers' command line in addition to the one set by the build system.")

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

   If ``CMAKE_BUILD_TYPE`` is ``Debug``, the default value is ``Info``,
   otherwise it is ``Notice``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_LOG_LEVEL
    HELP "Chooses the log level, which controls the amount of console log output compiled into the build."
    STRINGS "None" "Error" "Notice" "Warning" "Info" "Verbose")

target_compile_definitions(common
    INTERFACE
        "LOG_LEVEL=$<UPPER_CASE:LOG_LEVEL_${TFA_LOG_LEVEL}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_MEASURED_BOOT

   Enables support for the Measured Boot feature.

   Disabled by default. Forcibly disabled unless :ref:``TFA_TRUSTED_BOOT`` is
   enabled.
#]=======================================================================]

tfa_config_option(
    NAME TFA_MEASURED_BOOT
    HELP "Enable support for Measured Boot."
    DEPENDS TFA_TRUSTED_BOOT)

target_compile_definitions(common
    INTERFACE
        "MEASURED_BOOT=$<BOOL:${TFA_MEASURED_BOOT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_NON_TRUSTED_WORLD_KEY

   This option is used when ``GENERATE_COT=1``. It specifies the file that contains the Non-Trusted World private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.
#]=======================================================================]

tfa_config_option(
    NAME TFA_NON_TRUSTED_WORLD_KEY
    HELP "This option is used when ``GENERATE_COT=1``. It specifies the file that contains the Non-Trusted World private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_NS_BL2U

   Path to NS_BL2U image in the host file system. This image is optional. It is only needed if the platform makefile specifies that it is required in order to build the ``fwu_fip`` target.
#]=======================================================================]

tfa_config_option(
    NAME TFA_NS_BL2U
    HELP "Path to NS_BL2U image in the host file system. This image is optional. It is only needed if the platform makefile specifies that it is required in order to build the ``fwu_fip`` target.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_NS_TIMER_SWITCH

   Enable save and restore for non-secure timer register contents upon world switch. It can take either 0 (don't save and restore) or 1 (do save and restore). 0 is the default. An SPD may set this to 1 if it wants the timer registers to be saved and restored.
#]=======================================================================]

tfa_config_option(
    NAME TFA_NS_TIMER_SWITCH
    HELP "Enable save and restore for non-secure timer register contents upon world switch. It can take either 0 (don't save and restore) or 1 (do save and restore). 0 is the default. An SPD may set this to 1 if it wants the timer registers to be saved and restored.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_OVERRIDE_LIBC

   This option allows platforms to override the default libc for the BL image. It can be either 0 (include) or 1 (remove). The default value is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_OVERRIDE_LIBC
    HELP "This option allows platforms to override the default libc for the BL image. It can be either 0 (include) or 1 (remove). The default value is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PL011_GENERIC_UART

   Boolean option to indicate the PL011 driver that the underlying hardware is not a full PL011 UART but a minimally compliant generic UART, which is a subset of the PL011. The driver will not access any register that is not part of the SBSA generic UART specification. Default value is 0 (a full PL011 compliant UART is present).
#]=======================================================================]

tfa_config_option(
    NAME TFA_PL011_GENERIC_UART
    HELP "Boolean option to indicate the PL011 driver that the underlying hardware is not a full PL011 UART but a minimally compliant generic UART, which is a subset of the PL011. The driver will not access any register that is not part of the SBSA generic UART specification. Default value is 0 (a full PL011 compliant UART is present).")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PLAT

   Choose a platform to build TF-A for. The chosen platform name must be subdirectory of any depth under ``plat/``, and must contain a platform makefile named ``platform.mk``. For example, to build TF-A for the Arm Juno board, select PLAT=juno.
#]=======================================================================]

tfa_config_option(
    NAME TFA_PLAT
    HELP "Choose a platform to build TF-A for. The chosen platform name must be subdirectory of any depth under ``plat/``, and must contain a platform makefile named ``platform.mk``. For example, to build TF-A for the Arm Juno board, select PLAT=juno.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PRELOADED_BL33_BASE

   This option enables booting a preloaded BL33 image instead of the normal boot flow. When defined, it must specify the entry point address for the preloaded BL33 image. This option is incompatible with ``EL3_PAYLOAD_BASE``. If both are defined, ``EL3_PAYLOAD_BASE`` has priority over ``PRELOADED_BL33_BASE``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_PRELOADED_BL33_BASE
    HELP "This option enables booting a preloaded BL33 image instead of the normal boot flow. When defined, it must specify the entry point address for the preloaded BL33 image. This option is incompatible with ``EL3_PAYLOAD_BASE``. If both are defined, ``EL3_PAYLOAD_BASE`` has priority over ``PRELOADED_BL33_BASE``.")

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

tfa_config_option(
    NAME TFA_PROGRAMMABLE_RESET_ADDRESS
    HELP "Whether the reset vector address can be programmed.")

target_compile_definitions(common
    INTERFACE
        "PROGRAMMABLE_RESET_ADDRESS=$<BOOL:${TFA_PROGRAMMABLE_RESET_ADDRESS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_PSCI_EXTENDED_STATE_ID

   Per the PSCI 1.0 specification, there are two formats possible for the PSCI
   State-ID format: original and extended. When enabled, this option configures
   the generic PSCI layer to use the extended format.

   This option should be overridden by the platform, and it governs the return
   value of the ``PSCI_FEATURES`` API for the ``CPU_SUSPEND`` SMC function ID.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_PSCI_EXTENDED_STATE_ID
    HELP "Use the extended PSCI State-ID encoding."
    TYPE INTERNAL)

target_compile_definitions(common
    INTERFACE
        "PSCI_EXTENDED_STATE_ID=$<BOOL:${TFA_PSCI_EXTENDED_STATE_ID}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RAS_EXTENSION

   Enables RAS features. RAS is an optional extension for Armv8-A CPUs prior to
   Armv8.2-A, at which point it became mandatory.

   Disabled by default. Forcibly disabled if :ref:``TFA_HANDLE_EA_EL3_FIRST`` is
   disabled.
#]=======================================================================]

tfa_config_option(
    NAME TFA_RAS_EXTENSION
    HELP "Enable the Armv8-A RAS extension."
    DEPENDS TFA_HANDLE_EA_EL3_FIRST)

target_compile_definitions(common
    INTERFACE
        "RAS_EXTENSION=$<BOOL:${TFA_RAS_EXTENSION}>")

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

tfa_config_option(
    NAME TFA_RECLAIM_INIT_CODE
    HELP "Enable initialization code reclamation."
    DEPENDS NOT TFA_SEPARATE_NOBITS_REGION)

target_compile_definitions(common
    INTERFACE
        "RECLAIM_INIT_CODE=$<BOOL:${TFA_RECLAIM_INIT_CODE}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RESET_TO_BL31

   Use the BL31 entrypoint as the CPU reset vector instead of the BL1
   entrypoint.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_RESET_TO_BL31
    HELP "Reset into BL31 instead of BL1.")

target_compile_definitions(common
    INTERFACE
        "RESET_TO_BL31=$<BOOL:${TFA_RESET_TO_BL31}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RESET_TO_SP_MIN

   SP_MIN is the minimal AArch32 Secure Payload provided in TF-A. This flag configures SP_MIN entrypoint as the CPU reset vector instead of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1 entrypoint) or 1 (CPU reset to SP_MIN entrypoint). The default value is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_RESET_TO_SP_MIN
    HELP "SP_MIN is the minimal AArch32 Secure Payload provided in TF-A. This flag configures SP_MIN entrypoint as the CPU reset vector instead of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1 entrypoint) or 1 (CPU reset to SP_MIN entrypoint). The default value is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ROT_KEY

   This option is used when ``GENERATE_COT=1``. It specifies the file that contains the ROT private key in PEM format and enforces public key hash generation. If ``SAVE_KEYS=1``, this file name will be used to save the key.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ROT_KEY
    HELP "This option is used when ``GENERATE_COT=1``. It specifies the file that contains the ROT private key in PEM format and enforces public key hash generation. If ``SAVE_KEYS=1``, this file name will be used to save the key.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SAVE_KEYS

   This option is used when ``GENERATE_COT=1``. It tells the certificate generation tool to save the keys used to establish the Chain of Trust. Allowed options are '0' or '1'. Default is '0' (do not save).
#]=======================================================================]

tfa_config_option(
    NAME TFA_SAVE_KEYS
    HELP "This option is used when ``GENERATE_COT=1``. It tells the certificate generation tool to save the keys used to establish the Chain of Trust. Allowed options are '0' or '1'. Default is '0' (do not save).")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SCP_BL2

   Path to SCP_BL2 image in the host file system. This image is optional. If a SCP_BL2 image is present then this option must be passed for the ``fip`` target.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SCP_BL2
    HELP "Path to SCP_BL2 image in the host file system. This image is optional. If a SCP_BL2 image is present then this option must be passed for the ``fip`` target.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SCP_BL2_KEY

   This option is used when ``GENERATE_COT=1``. It specifies the file that contains the SCP_BL2 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SCP_BL2_KEY
    HELP "This option is used when ``GENERATE_COT=1``. It specifies the file that contains the SCP_BL2 private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SCP_BL2U

   Path to SCP_BL2U image in the host file system. This image is optional. It is only needed if the platform makefile specifies that it is required in order to build the ``fwu_fip`` target.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SCP_BL2U
    HELP "Path to SCP_BL2U image in the host file system. This image is optional. It is only needed if the platform makefile specifies that it is required in order to build the ``fwu_fip`` target.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SDEI_SUPPORT

   Enables support for the Software Delegated Exception Interface in BL31.

   Disabled by default. Forcibly disabled unless
   :ref:``TFA_EL3_EXCEPTION_HANDLING`` is enabled.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SDEI_SUPPORT
    HELP "Enables support for the Software Delegated Exception Interface in BL31."
    DEPENDS TFA_EL3_EXCEPTION_HANDLING)

target_compile_definitions(common
    INTERFACE
        "SDEI_SUPPORT=$<BOOL:${TFA_SDEI_SUPPORT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SEPARATE_CODE_AND_RODATA

   Whether code and read-only data should be isolated on separate memory pages.
   This is a trade-off between security and memory usage. See "Isolating code
   and read-only data on separate memory pages" section in
   :ref:`Firmware Design`.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SEPARATE_CODE_AND_RODATA
    HELP "Whether code and read-only data should be isolated on separate memory pages.")

target_compile_definitions(common
    INTERFACE
        "SEPARATE_CODE_AND_RODATA=$<BOOL:${TFA_SEPARATE_CODE_AND_RODATA}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SEPARATE_NOBITS_REGION

   Allows the NOBITS sections of BL31 (``.bss``, stacks, page tables, and
   coherent memory) to be allocated in RAM discontiguous from the loaded
   firmware image. When enabled, the platform is expected to provide definitions
   for ``BL31_NOBITS_BASE`` and ``BL31_NOBITS_LIMIT``. When disabled, ``NOBITS``
   sections are placed in RAM immediately following the loaded firmware image.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SEPARATE_NOBITS_REGION
    HELP "Allows BL31 NOBITS to be independently allocated.")

target_compile_definitions(common
    INTERFACE
        "SEPARATE_NOBITS_REGION=$<BOOL:${TFA_SEPARATE_NOBITS_REGION}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPD

   Choose a Secure Payload Dispatcher component to be built into TF-A. This build option is only valid if ``ARCH=aarch64``. The value should be the path to the directory containing the SPD source, relative to ``services/spd/``; the directory is expected to contain a makefile called ``<spd-value>.mk``. The SPM Dispatcher standard service is located in services/std_svc/spmd and enabled by ``SPD=spmd``. The SPM Dispatcher cannot be enabled when the ``SPM_MM`` option is enabled.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SPD
    HELP "Choose a Secure Payload Dispatcher component to be built into TF-A. This build option is only valid if ``ARCH=aarch64``. The value should be the path to the directory containing the SPD source, relative to ``services/spd/``; the directory is expected to contain a makefile called ``<spd-value>.mk``. The SPM Dispatcher standard service is located in services/std_svc/spmd and enabled by ``SPD=spmd``. The SPM Dispatcher cannot be enabled when the ``SPM_MM`` option is enabled.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPIN_ON_BL1_EXIT

   This option introduces an infinite loop in BL1. It can take either 0 (no loop) or 1 (add a loop). 0 is the default. This loop stops execution in BL1 just before handing over to BL31. At this point, all firmware images have been loaded in memory, and the MMU and caches are turned off. Refer to the "Debugging options" section for more details.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SPIN_ON_BL1_EXIT
    HELP "This option introduces an infinite loop in BL1. It can take either 0 (no loop) or 1 (add a loop). 0 is the default. This loop stops execution in BL1 just before handing over to BL31. At this point, all firmware images have been loaded in memory, and the MMU and caches are turned off. Refer to the \"Debugging options\" section for more details.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPMD_SPM_AT_SEL2

   this boolean option is used jointly with the SPM Dispatcher option (``SPD=spmd``). When enabled (1) it indicates the SPMC component runs at the S-EL2 execution state provided by the Armv8.4-SecEL2 extension. This is the default when enabling the SPM Dispatcher. When disabled (0) it indicates the SPMC component runs at the S-EL1 execution state. This latter configuration supports pre-Armv8.4 platforms (aka not implementing the Armv8.4-SecEL2 extension).
#]=======================================================================]

tfa_config_option(
    NAME TFA_SPMD_SPM_AT_SEL2
    HELP "this boolean option is used jointly with the SPM Dispatcher option (``SPD=spmd``). When enabled (1) it indicates the SPMC component runs at the S-EL2 execution state provided by the Armv8.4-SecEL2 extension. This is the default when enabling the SPM Dispatcher. When disabled (0) it indicates the SPMC component runs at the S-EL1 execution state. This latter configuration supports pre-Armv8.4 platforms (aka not implementing the Armv8.4-SecEL2 extension).")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SPM_MM

   Enable the Management Mode (MM)-based Secure Partition Manager (SPM)
   implementation.

   Disabled by default. Forcibly disabled if using the SPM Dispatcher, or if
   EL3 exceptions are disabled.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SPM_MM
    HELP "Enable the Management Mode (MM)-based Secure Partition Manager (SPM) implementation."
    DEPENDS TFA_EL3_EXCEPTION_HANDLING AND (NOT TFA_SPD STREQUAL "SPMD"))

target_compile_definitions(common
    INTERFACE
        "SPM_MM=$<BOOL:${TFA_SPM_MM}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SP_LAYOUT_FILE

   Platform provided path to JSON file containing the description of secure partitions. The build system will parse this file and package all secure partition blobs into the FIP. This file is not necessarily part of TF-A tree. Only available when ``SPD=spmd``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SP_LAYOUT_FILE
    HELP "Platform provided path to JSON file containing the description of secure partitions. The build system will parse this file and package all secure partition blobs into the FIP. This file is not necessarily part of TF-A tree. Only available when ``SPD=spmd``.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SP_MIN_WITH_SECURE_FIQ

   Boolean flag to indicate the SP_MIN handles secure interrupts (caught through the FIQ line). Platforms can enable this directive if they need to handle such interruption. When enabled, the FIQ are handled in monitor mode and non secure world is not allowed to mask these events. Platforms that enable FIQ handling in SP_MIN shall implement the api ``sp_min_plat_fiq_handler()``. The default value is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SP_MIN_WITH_SECURE_FIQ
    HELP "Boolean flag to indicate the SP_MIN handles secure interrupts (caught through the FIQ line). Platforms can enable this directive if they need to handle such interruption. When enabled, the FIQ are handled in monitor mode and non secure world is not allowed to mask these events. Platforms that enable FIQ handling in SP_MIN shall implement the api ``sp_min_plat_fiq_handler()``. The default value is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TRNG_SUPPORT

   Enables support for the True Random Number Generation interface.
#]=======================================================================]

tfa_config_option(
    NAME TFA_TRNG_SUPPORT
    HELP "Enable support for the True Random Number Generation interface.")

target_compile_definitions(common
    INTERFACE
        "TRNG_SUPPORT=$<BOOL:${TFA_TRNG_SUPPORT}>")

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

tfa_config_option(
    NAME TFA_TRUSTED_BOARD_BOOT
    HELP "Include support for the Trusted Board Boot feature.")

target_compile_definitions(common
    INTERFACE
        "TRUSTED_BOARD_BOOT=$<BOOL:${TFA_TRUSTED_BOARD_BOOT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TRUSTED_WORLD_KEY

   This option is used when ``GENERATE_COT=1``. It specifies the file that contains the Trusted World private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.
#]=======================================================================]

tfa_config_option(
    NAME TFA_TRUSTED_WORLD_KEY
    HELP "This option is used when ``GENERATE_COT=1``. It specifies the file that contains the Trusted World private key in PEM format. If ``SAVE_KEYS=1``, this file name will be used to save the key.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TSP_INIT_ASYNC

   Choose BL32 initialization method as asynchronous or synchronous, (see "Initializing a BL32 Image" section in :ref:`Firmware Design`). It can take the value 0 (BL32 is initialized using synchronous method) or 1 (BL32 is initialized using asynchronous method). Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_TSP_INIT_ASYNC
    HELP "Choose BL32 initialization method as asynchronous or synchronous, (see \"Initializing a BL32 Image\" section in :ref:`Firmware Design`). It can take the value 0 (BL32 is initialized using synchronous method) or 1 (BL32 is initialized using asynchronous method). Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_TSP_NS_INTR_ASYNC_PREEMPT

   A non zero value enables the interrupt routing model which routes non-secure interrupts asynchronously from TSP to EL3 causing immediate preemption of TSP. The EL3 is responsible for saving and restoring the TSP context in this routing model. The default routing model (when the value is 0) is to route non-secure interrupts to TSP allowing it to save its context and hand over synchronously to EL3 via an SMC.

   .. note::
      When ``EL3_EXCEPTION_HANDLING`` is ``1``, ``TSP_NS_INTR_ASYNC_PREEMPT`` must also be set to ``1``.
#]=======================================================================]

tfa_config_option(
    NAME TFA_TSP_NS_INTR_ASYNC_PREEMPT
    HELP "A non zero value enables the interrupt routing model which routes non-secure interrupts asynchronously from TSP to EL3 causing immediate preemption of TSP. The EL3 is responsible for saving and restoring the TSP context in this routing model. The default routing model (when the value is 0) is to route non-secure interrupts to TSP allowing it to save its context and hand over synchronously to EL3 via an SMC.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_ARM_LINK

   This flag determines whether to enable support for ARM linker. When the ``LINKER`` build variable points to the armlink linker, this flag is enabled automatically. To enable support for armlink, platforms will have to provide a scatter file for the BL image. Currently, Tegra platforms use the armlink support to compile BL3-1 images.
#]=======================================================================]

tfa_config_option(
    NAME TFA_USE_ARM_LINK
    HELP "This flag determines whether to enable support for ARM linker. When the ``LINKER`` build variable points to the armlink linker, this flag is enabled automatically. To enable support for armlink, platforms will have to provide a scatter file for the BL image. Currently, Tegra platforms use the armlink support to compile BL3-1 images.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_COHERENT_MEM

   This flag determines whether to include the coherent memory region in the BL
   memory map or not (see "Use of Coherent memory in TF-A" section in
   :ref:`Firmware Design`). If enabled, coherent memory region is included,
   otherwise coherent memory region is excluded.

   Enabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_USE_COHERENT_MEM
    HELP "Include the coherent memory region in the BL memory map."
    DEFAULT TRUE)

target_compile_definitions(common
    INTERFACE
        "USE_COHERENT_MEM=$<BOOL:${TFA_USE_COHERENT_MEM}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_DEBUGFS

   When enabled, this option activates a virtual filesystem interface through
   BL31 as a SiP SMC function.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_USE_DEBUGFS
    HELP "Enable DebugFS SiP SMC function.")

target_compile_definitions(common
    INTERFACE
        "USE_DEBUGFS=$<BOOL:${TFA_USE_DEBUGFS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_IO_IN_DTB

   This flag determines whether to use IO based on the firmware configuration framework. This will move the io_policies into a configuration device tree, instead of static structure in the code base. This is currently an experimental feature.
#]=======================================================================]

tfa_config_option(
    NAME TFA_ARM_IO_IN_DTB
    HELP "This flag determines whether to use IO based on the firmware configuration framework. This will move the io_policies into a configuration device tree, instead of static structure in the code base. This is currently an experimental feature.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_COT_DESC_IN_DTB

   This flag determines whether to create COT descriptors at runtime using fconf. If this flag is enabled, COT descriptors are statically captured in tb_fw_config file in the form of device tree nodes and properties. Currently, COT descriptors used by BL2 are moved to the device tree and COT descriptors used by BL1 are retained in the code base statically. This is currently an experimental feature.
#]=======================================================================]

tfa_config_option(
    NAME TFA_COT_DESC_IN_DTB
    HELP "This flag determines whether to create COT descriptors at runtime using fconf. If this flag is enabled, COT descriptors are statically captured in tb_fw_config file in the form of device tree nodes and properties. Currently, COT descriptors used by BL2 are moved to the device tree and COT descriptors used by BL1 are retained in the code base statically. This is currently an experimental feature.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SDEI_IN_FCONF

   Configures SDEI setup at runtime using the firmware configuration framework.
   The platform-specific SDEI shared and private events configuration is
   retrieved from the device tree rather than static C structures at compile
   time.

   Disabled by default. Forcibly disabled unless :ref:``TFA_SDEI_SUPPORT`` is
   enabled.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SDEI_IN_FCONF
    HELP "Configures SDEI setup at runtime using the firmware configuration framework."
    DEPENDS TFA_SDEI_SUPPORT)

target_compile_definitions(common
    INTERFACE
        "SDEI_IN_FCONF=$<BOOL:${TFA_SDEI_IN_FCONF}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SEC_INT_DESC_IN_FCONF

   This flag determines whether to configure Group 0 and Group1 secure interrupts using the firmware configuration framework. The platform specific secure interrupt property descriptor is retrieved from device tree in runtime rather than depending on static C structure at compile time. This is currently an experimental feature.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SEC_INT_DESC_IN_FCONF
    HELP "This flag determines whether to configure Group 0 and Group1 secure interrupts using the firmware configuration framework. The platform specific secure interrupt property descriptor is retrieved from device tree in runtime rather than depending on static C structure at compile time. This is currently an experimental feature.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_ROMLIB

   Enables support for Library at ROM. This feature creates a library of
   functions to be placed in ROM and thus reduces SRAM usage. Refer to
   :ref:`Library at ROM` for further details.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_USE_ROMLIB
    HELP "Enable Library at ROM support.")

target_compile_definitions(common
    INTERFACE
        "USE_ROMLIB=$<BOOL:${TFA_USE_ROMLIB}>")

#
# Pull in the platform targets. We'll need to do this first because the other
# binaries rely on some of the headers and sources provided by the platform.
#

add_subdirectory("${TFA_PLATFORM_SOURCE_DIR}" "${TFA_PLATFORM_BINARY_DIR}"
    EXCLUDE_FROM_ALL)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_V

   Verbose build. If assigned anything other than 0, the build commands are printed. Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_V
    HELP "Verbose build. If assigned anything other than 0, the build commands are printed. Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_VERSION_STRING

   String used in the log output for each TF-A image. Defaults to a string formed by concatenating the version number, build type and build string.
#]=======================================================================]

tfa_config_option(
    NAME TFA_VERSION_STRING
    HELP "String used in the log output for each TF-A image. Defaults to a string formed by concatenating the version number, build type and build string.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_W

   Warning level. Some compiler warning options of interest have been regrouped and put in the root Makefile. This flag can take the values 0 to 3, each level enabling more warning options. Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_W
    HELP "Warning level. Some compiler warning options of interest have been regrouped and put in the root Makefile. This flag can take the values 0 to 3, each level enabling more warning options. Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_WARMBOOT_ENABLE_DCACHE_EARLY

   Boolean option to enable D-cache early on the CPU after warm boot. This is applicable for platforms which do not require interconnect programming to enable cache coherency (eg: single cluster platforms). If this option is enabled, then warm boot path enables D-caches immediately after enabling MMU. This option defaults to 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_WARMBOOT_ENABLE_DCACHE_EARLY
    HELP "Boolean option to enable D-cache early on the CPU after warm boot. This is applicable for platforms which do not require interconnect programming to enable cache coherency (eg: single cluster platforms). If this option is enabled, then warm boot path enables D-caches immediately after enabling MMU. This option defaults to 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SUPPORT_STACK_MEMTAG

   This flag determines whether to enable memory tagging for stack or not. It accepts 2 values: ``yes`` and ``no``. The default value of this flag is ``no``. Note this option must be enabled only for ARM architecture greater than Armv8.5-A.
#]=======================================================================]

tfa_config_option(
    NAME TFA_SUPPORT_STACK_MEMTAG
    HELP "This flag determines whether to enable memory tagging for stack or not. It accepts 2 values: ``yes`` and ``no``. The default value of this flag is ``no``. Note this option must be enabled only for ARM architecture greater than Armv8.5-A.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_RAS_TRAP_LOWER_EL_ERR_ACCESS

   Enables the ``SCR_EL3.TERR`` bit to trap access to the RAS ``ERR`` and RAS
   ``ERX`` registers from lower ELs.

   Disabled by default.
#]=======================================================================]

tfa_config_option(
    NAME TFA_RAS_TRAP_LOWER_EL_ERR_ACCESS
    HELP "Enables RAS trapping from lower ELs.")

target_compile_definitions(common
    INTERFACE
        "RAS_TRAP_LOWER_EL_ERR_ACCESS=$<BOOL:${RAS_TRAP_LOWER_EL_ERR_ACCESS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_OPENSSL_DIR

   This flag is used to provide the installed openssl directory path on the host machine which is used to build certificate generation and firmware encryption tool.
#]=======================================================================]

tfa_config_option(
    NAME TFA_OPENSSL_DIR
    HELP "This flag is used to provide the installed openssl directory path on the host machine which is used to build certificate generation and firmware encryption tool.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_USE_SP804_TIMER

   Use the SP804 timer instead of the Generic Timer for functions that wait for an arbitrary time length (udelay and mdelay). The default value is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_USE_SP804_TIMER
    HELP "Use the SP804 timer instead of the Generic Timer for functions that wait for an arbitrary time length (udelay and mdelay). The default value is 0.")

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
