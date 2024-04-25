#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef build-config-mk
    build-config-mk := 1

    include $(dir $(lastword $(MAKEFILE_LIST)))utilities.mk

    #
    # Configure the base build directory. This is the directory in which all
    # build artefacts are generated.
    #

    build-base = $(or $(BUILD_BASE),build)

    #
    # Configure values that determine the subdirectory for build artefacts.
    # Generated build artefacts are placed in the subdirectory given by all the
    # values in this list joined by slashes.
    #
    # For example, if the value of `build-subdirs` is `a b c`, then build
    # artefacts are placed in `a/b/c`.
    #

    build-subdirs  = $(PLAT)
    build-subdirs += $(BUILD_TYPE)

    #
    # Generate the final build directories.
    #

    build-dir = $(build-base)/$(subst $(space),/,$(build-subdirs))
endif
