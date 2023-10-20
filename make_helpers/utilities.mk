#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Generate the power set of a set.
#
# This function takes the following arguments:
#
#   - $(1): the set to generate the power set of
#   - $(2): (optional, defaults to `:`) the delimiter to use for subsets
#
# Example usage:
#
#     $(call powerset,a b c,.) # a.b.c a.c a.b b.c c b a
#

powerset = $(strip $(call powerset1,$(1),$(or $(2),:)))
powerset1 = $(if $(1),$(call powerset2,$(firstword $(1)),$(filter-out $(firstword $(1)),$(1)),$(2)))
powerset2 = $(call powerset3,$(1),$(call powerset,$(2),$(3)),$(3))
powerset3 = $(foreach subset,$(2),$(1)$(3)$(subst $(space),$(3),$(subset))) $(1) $(2)

#
# Reverse a list.
#
# This function takes the following arguments:
#
#  - $(1): the list to reverse
#
# Example usage:
#
#     $(call reverse,a b c) # c b a
#

reverse = $(strip $(if $(1),$(call reverse1,$(1)) $(firstword $(1))))
reverse1 = $(call reverse,$(wordlist 2,$(words $(1)),$(1)))
