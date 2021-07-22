#[=======================================================================[.rst:
TFAMetadata
-----------

.. default-domain:: cmake

Metadata management utilities for TF-A.

TF-A uses a set of JSON-formatted metadata files to manage some of its
configuration data. This module provides a stable CMake interface for retrieving
values from these metadata files.

.. command:: tfa_platforms

.. code:: cmake

    tfa_platforms(<out-var>)

Return the list of supported platforms in ``<out-var>``.

.. command:: tfa_platform_path

.. code:: cmake

    tfa_platform_path(<out-var> PLATFORM <platform>)

Return the path to the platform ``<platform>`` in ``<out-var>``.
#]=======================================================================]

include_guard()

include(ArmAssert)
include(ArmExpand)

include(TFAJsonUtilities)

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

#
# External global metadata API.
#

tfa_json_getter(tfa_platforms
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_platforms
    DECODE MEMBERS)

tfa_json_getter(tfa_platform_path
    JSON "${_TFA_METADATA}" PARENT tfa_metadata_platforms_platform
    DECODE STRING)

