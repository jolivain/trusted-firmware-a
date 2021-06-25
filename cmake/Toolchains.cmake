#
# Specify the Arm architecture version we are targeting. The toolchain files use
# this information to determine the correct compiler flags.
#

include(TFAParseArchVersion)

#
# Configure the Arm architecture version. In the legacy build system the version
# parts were configured through `ARM_ARCH_MAJOR` and `ARM_ARCH_MINOR`. Here,
# we instead accept a string in the branded format - "ArmvX.Y-Z" - and derive
# the versions from that.
#

if(NOT TFA_PLATFORM_ARCH_VERSIONS)
    message(FATAL_ERROR
        "No supported Arm architecture versions defined!\n"

        "Please ensure your platform has provided a list of supported "
        "Arm architecture versions via `TFA_PLATFORM_ARCH_VERSIONS`.")
endif()

tfa_legacy_option(
    OLD ARM_ARCH_MAJOR ARM_ARCH_MINOR
    NEW TFA_ARCH_VERSION)

arm_config_option(
    NAME TFA_ARCH_VERSION
    HELP "Arm architecture version to target."
    DEFAULT ""
    STRINGS ${TFA_PLATFORM_ARCH_VERSIONS}
    DEPENDS NOT TFA_ARCH_VERSION IN_LIST TFA_PLATFORM_ARCH_VERSIONS
    ELSE "${TFA_ARCH_VERSION}")

tfa_parse_arch_version(
    VERSION "${TFA_ARCH_VERSION}"
    MAJOR TFA_ARCH_VERSION_MAJOR
    MINOR TFA_ARCH_VERSION_MINOR
    CLASS TFA_ARCH_VERSION_CLASS)

set(TFA_ARCH_VERSION_MAJOR "${TFA_ARCH_VERSION_MAJOR}" CACHE INTERNAL
    "Arm architecture version to target, major part.")
set(TFA_ARCH_VERSION_MINOR "${TFA_ARCH_VERSION_MINOR}" CACHE INTERNAL
    "Arm architecture version to target, minor part.")
set(TFA_ARCH_VERSION_CLASS "${TFA_ARCH_VERSION_CLASS}" CACHE INTERNAL
    "Arm architecture version to target, class part.")

string(TOLOWER ${TFA_ARCH_VERSION} TFA_ARCH_VERSION_UPPER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_VERSION_UPPER} TFA_ARCH_VERSION_UPPER_IDENT)
string(TOLOWER ${TFA_ARCH_VERSION} TFA_ARCH_VERSION_LOWER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_VERSION_LOWER} TFA_ARCH_VERSION_LOWER_IDENT)

#
# Configure the instruction set state. In the legacy build system this was done
# through `ARCH`, which took `aarch32` or `aarch64` regardless of the platform.
# We do something similar here, except we ask the platform to provide the list
# of architectures it supports if using an ISA supporting multiple instruction
# sets states.
#

if(TFA_ARCH_VERSION_MAJOR LESS 8)
    #
    # Prior to Armv8-A, everything was 32-bit only. While AArch32 is technically
    # an execution state introduced by Armv8-A, it is similar enough to Armv7-A
    # that we treat them synonymously. To that end, as far as we are concerned,
    # Armv7-A "supports" the AArch32 instruction set state.
    #

    if(NOT TFA_PLATFORM_ARCH_STATES STREQUAL "AArch32")
        message(WARNING
            "The platform-provided value for `TFA_PLATFORM_ARCH_STATES` has "
            "been overridden as the current target architecture is "
            "${TFA_ARCH_VERSION}, which only supports one state.")
    endif()

    set(TFA_PLATFORM_ARCH_STATES "AArch32")
endif()

if(NOT TFA_PLATFORM_ARCH_STATES)
    message(FATAL_ERROR
        "No supported instruction set states defined!\n"

        "Please ensure your platform has provided a list of supported "
        "instruction set states via `TFA_PLATFORM_ARCH_STATES`.")
endif()

tfa_legacy_option(
    OLD ARCH
    NEW TFA_ARCH_STATE)

arm_config_option(
    NAME TFA_ARCH_STATE
    HELP "Arm instruction set state."
    STRINGS ${TFA_PLATFORM_ARCH_STATES}
    DEFAULT "")

string(TOLOWER ${TFA_ARCH_STATE} TFA_ARCH_STATE_UPPER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_STATE_UPPER} TFA_ARCH_STATE_UPPER_IDENT)
string(TOLOWER ${TFA_ARCH_STATE} TFA_ARCH_STATE_LOWER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_STATE_LOWER} TFA_ARCH_STATE_LOWER_IDENT)

if(TFA_ARCH_STATE STREQUAL "AArch32")
    set(TFA_ARCH_STATE_AARCH32 TRUE CACHE INTERNAL "Are we targeting AArch32?")
    set(TFA_ARCH_STATE_AARCH64 FALSE CACHE INTERNAL "Are we targeting AArch64?")
else()
    set(TFA_ARCH_STATE_AARCH32 FALSE CACHE INTERNAL "Are we targeting AArch32?")
    set(TFA_ARCH_STATE_AARCH64 TRUE CACHE INTERNAL "Are we targeting AArch64?")
endif()

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
# Determine the toolchain file. A given platform must indicate support for at
# least one known toolchain file, even though the toolchain file can be provided
# manually by the user. This is predominantly so that the full build pipeline is
# reproducible in at least one fashion for every platform.
#
# From the user's perspective, toolchain files can be given through the standard
# CMake mechanisms (`CMAKE_TOOLCHAIN_FILE`), or they can select from a list of
# known-supported toolchains for the platform they're building.
#

if(NOT TFA_PLATFORM_TOOLCHAINS)
    message(FATAL_ERROR
        "No toolchains defined!\n"

        "Please ensure your platform has provided a list of supported "
        "toolchains via `TFA_PLATFORM_TOOLCHAINS`.")
endif()

arm_config_option(
    NAME TFA_TOOLCHAIN
    HELP "Toolchain name."
    STRINGS ${TFA_PLATFORM_TOOLCHAINS}
    DEFAULT ""
    DEPENDS (NOT TFA_TOOLCHAIN IN_LIST TFA_PLATFORM_TOOLCHAINS) AND
            (NOT CMAKE_TOOLCHAIN_FILE)
    ELSE "${TFA_TOOLCHAIN}")

#
# Toolchain files can be loaded in one of three ways:
#
# - The user can specify a `CMAKE_TOOLCHAIN_FILE` themselves, as is often done
#   in other CMake projects intended to be cross-compiled. In this case, we do
#   nothing but let it happen.
# - Platforms can provide their own toolchain files in a `toolchains` directory.
#   These may augment the toolchain files we already provide in some way, or
#   they can be entirely independent.
# - Platforms can use the default toolchain files we provide in the platform
#   list file.
#
# This is the branch that finally decides which toolchain file is active.
#

if(TFA_TOOLCHAIN AND (NOT CMAKE_TOOLCHAIN_FILE))
    set(CMAKE_TOOLCHAIN_FILE
        "${TFA_PLATFORM_SOURCE_DIR}/toolchains/${TFA_TOOLCHAIN}.cmake")

    if(NOT EXISTS CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE
            "${CMAKE_SOURCE_DIR}/toolchains/${TFA_TOOLCHAIN}.cmake")
    endif()
endif()
