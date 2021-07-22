#[=======================================================================[.rst:
TFAMetadata
----------------

.. default-domain:: cmake

.. command:: tfa_json_encode_string

Encode a CMake string as JSON.

.. code:: cmake

    tfa_json_encode_string(<out-var> <string>)

This function takes an input string ``<string>>``, escapes it according to
section 9 of the ECMA-404 JSON Data Interchange Syntax specification, wraps
it in quotes (``"``), and writes the result to the variable ``<out-var>``.

The resulting value is suitable for insertion into a JSON object.

.. command:: tfa_json_decode_array

Decode a JSON array into a CMake list.

.. code:: cmake

    tfa_json_decode_array(<out-var> <array>)

This function takes an input JSON array ``<array>``, and returns a CMake list
in ``<out-var>`` representing the string entries.
#]=======================================================================]

include_guard()

include(ArmAssert)
include(ArmConfigOption)
include(ArmExpand)

#
# Read the global metadata file. This is a JSON-formatted file that contains
# information about the contents of the repository that are relevant to the
# build system.
#

arm_assert(CONDITION EXISTS "${CMAKE_SOURCE_DIR}/metadata.json")

file(READ "${CMAKE_SOURCE_DIR}/metadata.json" _TFA_METADATA)
arm_expand(OUTPUT _TFA_METADATA STRING "${_TFA_METADATA}")

#
# Allow the user to provide their own platform list metadata. This allows
# developers to use out-of-tree platforms (platforms that live outside of this
# repository). The platforms list given by this file is superimposed onto the
# global metadata file.
#

arm_config_option(
    NAME TFA_METADATA_PLATFORMS_PATH
    HELP "Path to an alternative platforms metadata file."
    TYPE FILEPATH ADVANCED)

if(TFA_METADATA_PLATFORMS_PATH)
    cmake_path(GET TFA_METADATA_PLATFORMS_PATH
        PARENT_PATH TFA_PLATFORMS_METADATA_DIR)

    arm_assert(
        CONDITION EXISTS "${TFA_METADATA_PLATFORMS_PATH}"
        MESSAGE "The platforms metadata file does not exist: ${TFA_METADATA_PLATFORMS_PATH}")

    file(READ "${TFA_METADATA_PLATFORMS_PATH}" platforms-metadata)
    arm_expand(OUTPUT platforms-metadata STRING "${platforms-metadata}")

    tfa_json_merge(_TFA_METADATA
        BOTTOM "${_TFA_METADATA}"
        BOTTOM_PATH "platforms"
        TOP "${platforms-metadata}")
endif()

#
# Internal global metadata API.
#

tfa_json_getter(tfa_metadata
    JSON "${_TFA_METADATA}")

tfa_json_getter(tfa_metadata_platforms
    JSON "${_TFA_METADATA}" PARENT tfa_metadata
    PATH "platforms")

tfa_json_getter(tfa_metadata_platforms_platform
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_platforms
    PATH "@PLATFORM@" ARGUMENTS PLATFORM
    ERROR_MESSAGE "No such platform: @PLATFORM@.")

tfa_json_getter(tfa_metadata_architectures
    JSON "${_TFA_METADATA}" PARENT tfa_metadata
    PATH "architectures")

tfa_json_getter(tfa_metadata_architectures_architecture
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures
    PATH "@ARCHITECTURE@" ARGUMENTS ARCHITECTURE
    ERROR_MESSAGE "No such architecture: @ARCHITECTURE@.")

tfa_json_getter(tfa_metadata_architectures_architecture_states
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures_architecture
    PATH "states" DEFAULT "[]")

tfa_json_getter(tfa_metadata_architectures_architecture_features
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures_architecture
    PATH "features" DEFAULT "{}")

tfa_json_getter(tfa_metadata_architectures_architecture_features_mandatory
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures_architecture_features
    PATH "mandatory" DEFAULT "[]")

tfa_json_getter(tfa_metadata_architectures_architecture_features_optional
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures_architecture_features
    PATH "optional" DEFAULT "[]")

#
# External global metadata API.
#

tfa_json_getter(tfa_platforms
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_platforms
    DECODE MEMBERS)

tfa_json_getter(tfa_platform_path
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_platforms_platform
    DECODE STRING)

tfa_json_getter(tfa_architectures
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures
    DECODE MEMBERS)

tfa_json_getter(tfa_architecture_states
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures_architecture_states
    DECODE ARRAY)

tfa_json_getter(tfa_architecture_mandatory_features
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures_architecture_features_mandatory
    DECODE ARRAY)

tfa_json_getter(tfa_architecture_optional_features
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_architectures_architecture_features_optional
    DECODE ARRAY)

#
# Internal platform metadata API.
#

macro(tfa_platform_metadata_preprocess)
    #
    # Because we are using `tfa_platform_path` as our parent, `ARG_JSON` already
    # contains the decoded platform path.
    #

    cmake_path(ABSOLUTE_PATH ARG_JSON
        BASE_DIRECTORY "${CMAKE_SOURCE_DIR}")

    arm_assert(
        CONDITION EXISTS "${ARG_JSON}/platform.json"
        MESSAGE "The platform metadata file for the ${ARG_PLATFORM} platform "
                "could not be found: ${ARG_JSON}/platform.json")

    #
    # Replace `ARG_JSON` with the contents of the platform metadata file.
    #

    file(READ "${ARG_JSON}/platform.json" ARG_JSON)
    arm_expand(OUTPUT ARG_JSON STRING "${ARG_JSON}")
endmacro()

tfa_json_getter(tfa_platform_metadata
    JSON "${_TFA_METADATA}" PARENT tfa_platform_path
    PREPROCESS tfa_platform_metadata_preprocess)

tfa_json_getter(tfa_platform_metadata_target
    JSON "${_TFA_METADATA}" PARENT tfa_platform_metadata
    PATH "target")

tfa_json_getter(tfa_platform_metadata_architectures
    JSON "${_TFA_METADATA}" PARENT tfa_platform_metadata
    PATH "architectures")

tfa_json_getter(tfa_platform_metadata_architectures_architecture
    JSON "${_TFA_METADATA}" PARENT tfa_platform_metadata_architectures
    PATH "@ARCHITECTURE@" ARGUMENTS ARCHITECTURE
    ERROR_MESSAGE "The platform @PLATFORM@ does not support the "
                  "@ARCHITECTURE@ architecture.")

tfa_json_getter(tfa_platform_metadata_architectures_architecture_states
    JSON "${_TFA_METADATA}" PARENT tfa_platform_metadata_architectures_architecture
    PATH "states" DEFAULT "[]")

#
# External platform metadata API.
#

tfa_json_getter(tfa_platform_target
    JSON "${_TFA_METADATA}" PARENT tfa_platform_metadata_target
    DECODE STRING)

tfa_json_getter(tfa_platform_architectures
    JSON "${_TFA_METADATA}" PARENT tfa_platform_metadata_architectures
    DECODE MEMBERS)

tfa_json_getter(tfa_platform_architecture_states
    JSON "${_TFA_METADATA}" PARENT tfa_platform_metadata_architectures_architecture_states
    DECODE ARRAY)
