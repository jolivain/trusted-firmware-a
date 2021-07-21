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

