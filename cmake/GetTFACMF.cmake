#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
GetTFACMF
=========

The TF-A CMake Framework Getter module is handling the |TFACMF| dependency. The
module uses a set of cache variables to control the following dependency
details:

   * TFACMF version to be looking for.
   * Git repository URL to fetch |TFACMF| from.

Cache variables can be set from the command line to override default settings.
Example:

.. code-block:: shell

   cmake -DTFACMF_VERSION="1.0.0"

TFACMF lookup process
---------------------

The framework lookup happens by executing the following steps:

  1. :cmake:command:`find_module` is called to check if the framework is
     available. :cmake:variable:`TFACMF_VERSION` specifies the version to be
     searching for.

    a. :cmake:command:`find_module` will look for :cmake:variable:`TFACMF_DIR`
       and if set, use the location defined to look for the package. If the
       package is found and the version is compatible the *search finishes*.

    b. :cmake:command:`find_module` will look for |TFACMF| in the
       `<CMAKE_BINARY_DIR>/cmake-framework`. If the package is found and the
       version is compatible the *search finishes*.

  2. :cmake:module:`FetchContent` is used to download |TFACMF| from git.
     :cmake:variable:`TFACMF_GIT_URL` and :cmake:variable:`TFACMF_VERSION` is
     used to specify from where and which tag to fetch.

    a. :cmake:command:`find_module` is called again to double check the fetched
       content.

Please find the same process visualised below.

.. uml:: /resources/diagrams/plantuml/cmake_tfacmf_lookup.puml

Overriding dependency settings
------------------------------

  - To use a different version:

    #. Set :cmake:variable:`TFACMF_VERSION` to control which version (tag)
       to fetch.

  - To use |TFACMF| from a different repository:

    #. Set :cmake:variable:`TFACMF_GIT_URL` to point to the right repository.
    #. Set :cmake:variable:`TFACMF_VERSION` to control which version (tag)
       to fetch.

  - To use any version:

    #. Clone |TFACMF| to a directory. Check out the version which is to be used
       and modify the content as needed.
    #. Set :cmake:variable:`TFACMF_VERSION` and :cmake:variable:`TFACMF_DIR` to
       force the build system to use the prepared instance.

Reference
---------

.. cmake:variable:: TFACMF_OFFICIAL_VERSION

  The |TFACMF| version which is officially supported by the current build
  definitions.

.. cmake:variable:: TFACMF_VERSION

  |TFACMF| version the search will look for.

.. cmake:variable:: TFACMF_GIT_URL

  Location of git repository where |TFACMF| is fetched from. The global git
  configuration must allow password-less access to the repository. This module
  does not support specifying credentials.

#]===]

# Set required CMake version
cmake_minimum_required(VERSION 3.14)

# Set TFACMF version to look for. This variable captures the "official" version
# the build depends on.
set(TFACMF_OFFICIAL_VERSION "0.1.0" CACHE STRING "Required version of TFACMF."
	FORCE)

# This is the version of TFACMF the build definition will look for. This can be
# overridden from the command line.
set(TFACMF_VERSION "${TFACMF_OFFICIAL_VERSION}" CACHE STRING
	"Actual version of TFACMF.")

# The repository URL to fetch TFACMF from.
set(TFACMF_GIT_URL
	"https://git.trustedfirmware.org/TF-A/cmake-framework.git"
	CACHE STRING "Git repo URL to fetch TFACMF from.")

if(NOT TFACMF_VERSION STREQUAL TFACMF_OFFICIAL_VERSION)
	message(STATUS "Dependency TFACMF configured to use version '${TFACMF_VERSION}' instead of '${TFACMF_OFFICIAL_VERSION}'.")
endif()

# Find the framework
find_package(TFACMF ${TFACMF_VERSION} PATHS ${CMAKE_BINARY_DIR}/cmake-framework)

if(NOT TFACMF_FOUND EQUAL 1)
	message(STATUS "Fetching TFACMF v${TFACMF_VERSION} from ${TFACMF_GIT_URL}.")
	include(FetchContent)
	FetchContent_Declare(
		tfacmf
		GIT_REPOSITORY ${TFACMF_GIT_URL}
		GIT_TAG "v${TFACMF_VERSION}"
		SOURCE_DIR ${CMAKE_BINARY_DIR}/cmake-framework
	)
	FetchContent_MakeAvailable(tfacmf)
	find_package(TFACMF ${TFACMF_VERSION} REQUIRED PATHS ${tfacmf_SOURCE_DIR})
endif()

message(STATUS "Using TFACMF v${TFACMF_VERSION} from ${TFACMF_DIR}")
