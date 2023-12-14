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

#
# Check that two string or integer values are exactly equal.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# If the two values are equal, the left hand side value is returned.
#
# Example usage:
#
#     $(call eq,4,4) # 4
#     $(call eq,4,5) # <nothing>
#
#     $(call eq,hello world!,hello world!) # hello world!
#     $(call eq,hello world!,hello world?) # <nothing>
#
# Be aware that this function is lossy, and spaces are not considered in the
# comparison.
#

eq = $(if $(filter $(call merge,$(1)),$(call merge,$(2))),$(1))

#
# Check that two string or integer values are not exactly equal.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# If the two values are not equal, the left hand side value is returned.
#
# Example usage:
#
#     $(call ne,4,4) # <nothing>
#     $(call ne,4,5) # 4
#
#     $(call ne,hello world!,hello world!) # <nothing>
#     $(call ne,hello world!,hello world?) # hello world!
#

ne = $(if $(filter-out $(call merge,$(2)),$(call merge,$(1))),$(1))

#
# Check that an integer value is less than another.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# If the left hand side value is less than the right hand side value then the
# left hand side value is returned.
#
# Example usage:
#
#     $(call lt,4,5) # 4
#     $(call lt,5,5) # <nothing>
#     $(call lt,6,5) # <nothing>
#

lt = $(and $(call lte,$(1),$(2)),$(call ne,$(1),$(2)))

#
# Check that an integer value is less than or equal to another.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# If the left hand side value is less than or equal to the right hand side value
# then the left hand side value is returned.
#
# Example usage:
#
#     $(call lte,4,5) # 4
#     $(call lte,5,5) # 5
#     $(call lte,6,5) # <nothing>
#

lte = $(filter $(firstword $(sort $(1) $(2))),$(1))

#
# Check that an integer value is greater than another.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# If the left hand side value is greater than the right hand side value then
# the left hand side value is returned.
#
# Example usage:
#
#     $(call gt,4,5) # <nothing>
#     $(call gt,5,5) # <nothing>
#     $(call gt,6,5) # 6
#

gt = $(and $(call gte,$(1),$(2)),$(call ne,$(1),$(2)))

#
# Check that an integer value is greater than or equal to another.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# If the left hand side value is greater than or equal to the right hand side
# value then the left hand side value is returned.
#
# Example usage:
#
#     $(call gte,4,5) # <nothing>
#     $(call gte,5,5) # 5
#     $(call gte,6,5) # 6
#

gte = $(filter $(lastword $(sort $(1) $(2))),$(1))

#
# Generate a sequence of integer values starting from zero.
#
# This function takes the following arguments:
#
#   - $(1): the maximum value
#   - $(2): the intermediate sequence to resume (used internally)
#
# Example usage:
#
#     $(call sequence0,0) # 0
#     $(call sequence0,1) # 0 1
#     $(call sequence0,3) # 0 1 2 3
#     $(call sequence0,5) # 0 1 2 3 4 5
#
# This function uses recursion to generate the sequence, and is therefore not
# suitable for operations on large values.
#

sequence0 = $(strip $(if $(call eq,$(lastword $(2)),$(1)),$(2), \
        $(call sequence0,$(1),$(2) $(words $(2)))))

#
# Generate a sequence of integer values starting from one.
#
# This function takes the following arguments:
#
#   - $(1): the maximum value
#
# Example usage:
#
#     $(call sequence1,0) #
#     $(call sequence1,1) # 1
#     $(call sequence1,3) # 1 2 3
#     $(call sequence1,5) # 1 2 3 4 5
#
# This function uses recursion to generate the sequence, and is therefore not
# suitable for operations on large values.
#

sequence1 = $(filter-out 0,$(call sequence0,$(1)))

#
# Increment an integer value by one.
#
# This function takes the following arguments:
#
#   - $(1): the value to increment
#
# Example usage:
#
#     $(call increment,0) # 1
#     $(call increment,1) # 2
#     $(call increment,3) # 4
#     $(call increment,5) # 6
#
# This function uses recursive sequence generation to emulate arithmetic
# operations, and is therefore not suitable for operations on large values.
#

increment = $(words $(call sequence0,$(1)))

#
# Decrement an integer value by one.
#
# This function takes the following arguments:
#
#   - $(1): the value to decrement
#
# Example usage:
#
#     $(call decrement,0) # <nothing>
#     $(call decrement,1) # 0
#     $(call decrement,3) # 2
#     $(call decrement,5) # 4
#
# Note that a negative result yields no value.
#
# This function uses recursive sequence generation to emulate arithmetic
# operations, and is therefore not suitable for operations on large values.
#

decrement = $(strip $(if $(call ne,$(1),0), \
        $(call nth,$(call sequence0,$(1)),$(1))))

#
# Reverse a sequence.
#
# This function takes the following arguments:
#
#  - $(1): the sequence to reverse
#
# Example usage:
#
#     $(call reverse,a) # a
#     $(call reverse,a b) # b a
#     $(call reverse,a b c) # c b a
#
# Internally, this function operates recursively, and is therefore not suitable
# for operations on very large sequences.
#

reverse = $(strip $(if $(1), \
        $(call reverse,$(wordlist 2,$(words $(1)),$(1))) \
        $(firstword $(1))))
