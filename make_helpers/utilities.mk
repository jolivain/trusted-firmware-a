#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

empty :=
space := $(empty) $(empty)
comma := ,

#
# Merge a list into a single string.
#
# This function takes the following arguments:
#
#   - $(1): the list to merge
#   - $(2): the delimiter to use between elements (optional)
#
# Example usage:
#
#     $(call merge,a b c) # abc
#     $(call merge,a b c,:) # a:b:c
#
# This is equivalent to doing a string substitution of all spaces in the list
# variable with the delimiter.
#

merge = $(subst $(space),$(2),$(strip $(1)))

#
# Split a string at a delimiter.
#
# This function takes the following arguments:
#
#   - $(1): the string to split
#   - $(2): the delimiter to split at
#
# Example usage:
#
#     $(call split,a-b-c,-) # a b c
#     $(call split,a:b:c,:) # a b c
#
# This is equivalent to doing a string substitution of the delimiter with a
# single space.
#

split = $(subst $(2),$(space),$(1))

#
# Retrieve the nth word(s) from a list.
#
# This function takes the following arguments:
#
#   - $(1): a list of words
#   - $(2): a list of word indices to read
#
# Example usage:
#
#     $(call nth,a b c,2) # b
#     $(call nth,a b c,1 3) # a c
#

nth = $(foreach i,$(2),$(wordlist $(i),$(i),$(1)))
