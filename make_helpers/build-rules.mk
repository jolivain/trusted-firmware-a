#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef build-rules-mk
        build-rules-mk := $(lastword $(MAKEFILE_LIST))

        .SUFFIXES:

        .PHONY: all clean distclean realclean

        .DEFAULT_GOAL := all

        .PHONY: FORCE
        FORCE: ;
endif
