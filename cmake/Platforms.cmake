#
# Determine whether this is an out-of-tree build based on whether the user has
# given us a platform source directory.
#

if(TFA_PLATFORM_SOURCE_DIR)
    set(TFA_PLATFORM_EXTERNAL TRUE)
endif()

#
# Gather the platform paths that we maintain within the repository. We do this
# to allow users to use `TFA_PLATFORM` to choose in-tree platforms instead of
# `TFA_PLATFORM_SOURCE_DIR`.
#
# If we're being asked to build an out-of-tree platform, we need to load the
# platform metadata file early to figure it out. We use a platform list limited
# to the single platform we're being asked to use.
#

if(TFA_PLATFORM_EXTERNAL)
    include("${TFA_PLATFORM_SOURCE_DIR}/Platform.cmake" OPTIONAL
        RESULT_VARIABLE exists)

    if(NOT exists)
        message(FATAL_ERROR
            "Missing platform metadata file!\n"

            "${TFA_PLATFORM_SOURCE_DIR}/Platform.cmake\n"

            "Out-of-tree platforms must provide a platform metadata file. "
            "Refer to the build system documentation for instructions on "
            "setting up a new platform.")
    endif()

    if(NOT TFA_PLATFORM)
        message(FATAL_ERROR
            "Missing platform name!\n"

            "${TFA_PLATFORM_SOURCE_DIR}/Platform.cmake\n"

            "Out-of-tree platforms must provide a platform name. You can do so "
            "by setting `TFA_PLATFORM` in your `Platform.cmake` file.")
    endif()

    list(APPEND valid_platforms "${TFA_PLATFORM}")
    list(APPEND valid_platform_source_dirs "${TFA_PLATFORM_SOURCE_DIR}")
else()
    list(APPEND valid_platforms "FVP")
    list(APPEND valid_platform_source_dirs "plat/arm/board/fvp")
endif()

#
# The platform name configuration option has some unusual requirements. It needs
# to reject any invalid platform names, and once it's been locked in it can no
# longer be changed because we risk cross-contamination of cache variables, and
# because the toolchain files can only be loaded once (and they may differ from
# platform to platform).
#
# To meet the requirements for this option, we need to give it an invalid
# default value (an empty string), then give it a dependency that can only
# be met if it's invalid, and finally we set the else value to itself such
# that once the dependency is no longer met (i.e. it's a value value) then
# it will not be reset to its default.
#

arm_config_option(
    NAME TFA_PLATFORM
    HELP "Platform to build."
    STRINGS ${valid_platforms}
    DEFAULT "" # Force a configuration failure if unset
    DEPENDS NOT TFA_PLATFORM IN_LIST valid_platforms
    ELSE "${TFA_PLATFORM}") # Maintain the value when it's forcibly hidden

string(TOLOWER ${TFA_PLATFORM} TFA_PLATFORM_UPPER)
string(MAKE_C_IDENTIFIER ${TFA_PLATFORM_UPPER} TFA_PLATFORM_UPPER_IDENT)
string(TOLOWER ${TFA_PLATFORM} TFA_PLATFORM_LOWER)
string(MAKE_C_IDENTIFIER ${TFA_PLATFORM_LOWER} TFA_PLATFORM_LOWER_IDENT)

#
# Derive the platform source directory from the platform name.
#

if(NOT TFA_PLATFORM_EXTERNAL)
    list(FIND valid_platforms "${TFA_PLATFORM}" idx)
    list(GET valid_platform_source_dirs "${idx}" TFA_PLATFORM_SOURCE_DIR)
endif()

#
# Generate the platform binary directory. We do this by converting the platform
# name to a C identifier (which should get rid of any non-portable characters),
# then converting it to lower-case to match the typical naming conventions.
#

set(TFA_PLATFORM_BINARY_DIR "plat/${TFA_PLATFORM_LOWER_IDENT}")

#
# Load in the platform metadata file for in-tree platforms (it's already been
# done for out-of-tree platforms).
#

if(NOT TFA_PLATFORM_EXTERNAL)
    include("${TFA_PLATFORM_SOURCE_DIR}/Platform.cmake")
endif()
