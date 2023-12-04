#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
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
# Add an integer value to another.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# Example usage:
#
#     $(call add,0,1) # 1
#     $(call add,1,3) # 4
#     $(call add,3,5) # 8
#
# Internally, this function operates recursively on generated sequences of
# positive integer values, and is therefore not suitable for operations on large
# or negative values.
#

add = $(strip $(if $(call eq,0,$(2)),$(1), \
        $(call add,$(call increment,$(1)),$(call decrement,$(2)))))

#
# Subtract an integer value from another.
#
# This function takes the following arguments:
#
#   - $(1): the left hand side value
#   - $(2): the right hand side value
#
# Example usage:
#
#     $(call sub,0,1) # <nothing>
#     $(call sub,0,0) # 0
#     $(call sub,1,0) # 1
#     $(call sub,3,1) # 2
#     $(call sub,5,3) # 2
#
# Note that a negative result yields no value.
#
# Internally, this function operates recursively on generated sequences of
# positive integer values, and is therefore not suitable for operations on large
# or negative values.
#

sub = $(strip $(if $(call eq,0,$(2)),$(1), \
        $(call sub,$(call decrement,$(1)),$(call decrement,$(2)))))

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

#
# Generate the cartesian product of a sequence with itself.
#
# This function takes the following arguments:
#
#   - $(1): the sequence to generate the product of
#   - $(2): a delimiter to merge subsequences using (optional)
#   - $(3): the number of repetitions of the input sequence (optional)
#
# If no repetition count is provided, it defaults to the number of elements in
# the input sequence.
#
# Example usage:
#
#     $(call product,a b c) # aaa aab aac aba abb abc aca acb acc baa bab bac...
#     $(call product,a b c,:) # a:a:a a:a:b a:a:c a:b:a a:b:b a:b:c a:c:a...
#     $(call product,a b c,,2) # aa ab ac ba bb bc ca cb cc
#     $(call product,a b c,:,2) # a:a a:b a:c b:a b:b b:c c:a c:b c:c
#
# Internally, this function operates recursively, and is therefore not suitable
# for operations on large sequences.
#

product = $(strip $(if $(call lte,$(3),1),$(1), \
        $(call product-pairs,$(1),$(call product,$(1),$(2), \
                $(call decrement,$(or $(3),$(words $(1))))),$(2))))

#
# Generate the cartesian product of two sequences.
#
# This function takes the following arguments:
#
#   - $(1): the left hand list to generate the product of
#   - $(2): the right hand list to generate the product of
#   - $(3): a delimiter to merge subsequences using (optional)
#
# Example usage:
#
#     $(call product-pairs,a b c d,1 2) # a1 a2 b1 b2 c1 c2 d1 d2
#     $(call product-pairs,a b c d,1 2,:) # a:1 a:2 b:1 b:2 c:1 c:2 d:1 d:2
#
# Internally, this function operates recursively, and is therefore not suitable
# for operations on large sequences.
#

product-pairs = $(strip $(foreach a,$(1),$(foreach b,$(2),$(a)$(3)$(b))))

#
# Generate permutations of a sequence.
#
# This function takes the following arguments:
#
#   - $(1): the sequence to permute
#   - $(3): a delimiter to merge subsequences using (optional)
#   - $(3): the number of repetitions of the sequence (optional)
#
# If no repetition count is provided, it defaults to the number of elements in
# the input sequence.
#
# Example usage:
#
#     $(call permutations,a b c) # abc acb bac bca cab cba
#     $(call permutations,a b c,:) # a:b:c a:c:b b:a:c b:c:a c:a:b c:b:a
#     $(call permutations,a b c,:,2) # a:b a:c b:a b:c c:a c:b.
#
# Internally, this function operates recursively, and is therefore not suitable
# for operations on large sequences.
#

permutations = $(strip $(foreach range, \
        $(call product,$(call sequence1,$(words $(1))),:, \
                $(or $(3),$(words $(1)))), \
        $(call permutations-subsequence,$(1), \
                $(call split,$(range),:),$(2),$(or $(3),$(words $(1))))))

permutations-subsequence = $(strip $(if \
        $(call eq,$(words $(2)),$(words $(sort $(2)))), \
        $(call merge,$(call nth,$(1),$(2),$(3)),$(3))))

#
# Generate subsequences of a list with with a specific length.
#
# This function takes the following arguments:
#
#   - $(1): the list to generate subsequences from
#   - $(2): the length of the subsequences returned
#   - $(3): a delimiter to merge subsequences using
#
# Example usage:
#
#     $(call combinations,a b c d,1) # a b c d
#     $(call combinations,a b c d,1,:) # a b c d
#     $(call combinations,a b c d,2) # ab ac ad bc bd cd
#     $(call combinations,a b c d,2,:) # a:b a:c a:d b:c b:d c:d
#     $(call combinations,a b c d,3) # abc abd acd bcd
#     $(call combinations,a b c d,3,:) # a:b:c a:b:d a:c:d b:c:d
#     $(call combinations,a b c d,4) # abcd
#     $(call combinations,a b c d,4,:) # a:b:c:d
#

combinations = $(strip $(foreach range, \
        $(call permutations,$(call sequence1,$(words $(1))),:,$(2)), \
        $(call combinations-subsequence,$(1), \
                $(call split,$(range),:),$(2),$(3))))
combinations-subsequence = $(strip $(if $(call eq,$(sort $(2)),$(2)), \
        $(call merge,$(call nth,$(1),$(2)),$(4))))

#
# Generate the power set of a set.
#
# This function takes the following arguments:
#
#   - $(1): the set to generate the power set of
#   - $(2): a delimiter to merge subsequences using (optional)
#
# Example usage:
#
#     $(call powerset,a b c) # a b c ab ac bc abc
#     $(call powerset,a b c,:) # a b c a:b a:c b:c a:b:c
#

powerset = $(call powerset-ascending,$(1),$(2))

#
# Generate the power set of a set in ascending order of subset length.
#
# This function takes the following arguments:
#
#   - $(1): the set to generate the power set of
#   - $(2): a delimiter to merge subsets using (optional)
#
# Example usage:
#
#     $(call powerset-ascending,a b c) # a b c ab ac bc abc
#     $(call powerset-ascending,a b c,:) # a b c a:b a:c b:c a:b:c
#

powerset-ascending = $(strip $(foreach i,
        $(call sequence1,$(words $(1))), \
        $(call combinations,$(1),$(i),$(2))))

#
# Generate the power set of a set in descending order of subset length.
#
# This function takes the following arguments:
#
#   - $(1): the set to generate the power set of
#   - $(2): a delimiter to merge subsets using (optional)
#
# Example usage:
#
#     $(call powerset-descending,a b c) # abc ab ac bc a b c
#     $(call powerset-descending,a b c,:) # a:b:c a:b a:c b:c a b c
#

powerset-descending = $(strip $(foreach i, \
        $(call reverse,$(call sequence1,$(words $(1)))), \
        $(call combinations,$(1),$(i),$(2))))
