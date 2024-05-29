#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

space :=
space := $(space) $(space)
comma := ,

null := �

compat-path = $(subst $(space),$(null),$(1))
decompat-path = $(subst $(null), ,$(1))

absolute-path = $(call decompat-path,$(abspath $(call compat-path,$(1))))
real-path = $(call decompat-path,$(realpath $(call compat-path,$(1))))

file-name = $(call decompat-path,$(notdir $(call compat-path,$(1))))
directory-name = $(call decompat-path,$(dir $(call compat-path,$(1))))

escape-shell = '$(subst ','\'',$(1))'

#
# Upper-case a string value.
#
# Parameters:
#
#   - $(1): string to upper-case
#
# Example usage:
#
#     $(call upper-case,HeLlO wOrLd) # "HELLO WORLD"
#

upper-case = $(shell echo $(call escape-shell,$(1)) | tr '[:lower:]' '[:upper:]')

#
# Lower-case a string value.
#
# Parameters:
#
#   - $(1): string to lower-case
#
# Example usage:
#
#     $(call lower-case,HeLlO wOrLd) # "hello world"
#

lower-case = $(shell echo $(call escape-shell,$(1)) | tr '[:upper:]' '[:lower:]')

#
# Determine the "truthiness" of a value.
#
# Parameters:
#
#   - $(1): value to determine the truthiness of
#
# A value is considered truthy if:
#
#   - it is not 0, and
#   - it is not equal to 'N' when upper-cased, and
#   - it is not equal to 'NO' when upper-cased, and
#   - it is not equal to 'F' when upper-cased, and
#   - it is not equal to 'FALSE' when upper-cased.
#
# If the value is truthy then the value is returned as-is, otherwise no value
# is returned. This makes it suitable for use in `$(if)` expressions, e.g.:
#
#     hello-world := 1
#
#     $(if $(call bool,$(hello-world),$(error hello, world!)) # hello, world!"
#

bool = $(filter-out 0 n no f false,$(call lower-case,$(1)))
