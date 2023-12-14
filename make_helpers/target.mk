#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef target-mk
        target-mk := $(lastword $(MAKEFILE_LIST))

        include $(dir $(target-mk))toolchain.mk
        include $(dir $(target-mk))utilities.mk

        #
        # Retrieve properties of a target.
        #
        # Properties are arbitrary values associated with a target, typically a
        # build artifact like a library or binary.
        #
        # Parameters:
        #
        #   - $(1): properties to read (e.g. `flags`, `definitions`)
        #   - $(2): toolchain to use (e.g. `aarch64`, `aarch32`, `host`)
        #   - $(3): tool class to use (e.g. `cc`, `ld`, `od`)
        #   - $(4): target identifier [optional] (e.g. `bl1`, `bl31`, `romlib`)
        #
        # For each target property requested, the power set of the remaining
        # parameters generates variables that contribute to the value returned:
        #
        #   - `<property>`
        #   - `<toolchain>-<property>`
        #   - `<tool>-<property>`
        #   - `<tool-class>-<property>`
        #   - `<toolchain>-<tool>-<property>`
        #   - `<toolchain>-<tool-class>-id-<property>`
        #   - `<tool>-<tool-class>-<property>`
        #   - `<toolchain>-<tool>-<tool-class>-<property>`
        #   - `<target>-<property>`
        #   - `<target>-<toolchain>-<property>`
        #   - `<target>-<tool>-<property>`
        #   - `<target>-<tool-class>-<property>`
        #   - `<target>-<toolchain>-<tool>-<property>`
        #   - `<target>-<tool>-<property>`
        #   - `<target>-<tool>-<tool-class>-<property>`
        #   - `<target>-<toolchain>-<tool>-<tool-class>-<property>`
        #
        # The rest of the build system can then append values to any of these
        # variables, and they will be returned by any matching call.
        #
        # The tool identifier is derived automatically from the expansion of the
        # variable `<toolchain>-<tool-class>-id`. For example, in the following
        # usage of the function:
        #
        #     $(call target-properties,flags,aarch64,cc,bl2)
        #
        # The following variables contribute to the `flags` target property for
        # the `bl2` target when `$(aarch64-cc-id)` is `gnu-gcc`:
        #
        #   - `flags`
        #   - `aarch64-flags`
        #   - `gnu-gcc-flags`
        #   - `cc-flags`
        #   - `aarch64-gnu-gcc-flags`
        #   - `aarch64-cc-id-flags`
        #   - `gnu-gcc-cc-flags`
        #   - `aarch64-gnu-gcc-cc-flags`
        #   - `bl2-flags`
        #   - `bl2-aarch64-flags`
        #   - `bl2-gnu-gcc-flags`
        #   - `bl2-cc-flags`
        #   - `bl2-aarch64-gnu-gcc-flags`
        #   - `bl2-gnu-gcc-flags`
        #   - `bl2-gnu-gcc-cc-flags`
        #   - `bl2-aarch64-gnu-gcc-cc-flags`
        #
        # These variables are passed the same arguments as the arguments that
        # `target-properties` receives, in the same order.
        #

        target-properties = $(foreach variable, \
                $(call filter-defined, \
                        $(call target-properties-powerset,$(1),$(2),$(3),$(4))),$\
                $(call $(variable),$(1),$(2),$(3),$(4)))

        target-properties-powerset = $(strip $(foreach property,$(1),$\
                $(call target-powerset,$(property),$(2),$(3),$(4))))

        target-powerset  = $(1)
        target-powerset += $(2)-$(1)
        target-powerset += $($(2)-$(3)-id)-$(1)
        target-powerset += $(3)-$(1)
        target-powerset += $(2)-$($(2)-$(3)-id)-$(1)
        target-powerset += $(2)-$(3)-$(1)
        target-powerset += $($(2)-$(3)-id)-$(3)-$(1)
        target-powerset += $(2)-$($(2)-$(3)-id)-$(3)-$(1)

        target-powerset += $(if $(4),$(4)-$(1))
        target-powerset += $(if $(4),$(4)-$(2)-$(1))
        target-powerset += $(if $(4),$(4)-$($(2)-$(3)-id)-$(1))
        target-powerset += $(if $(4),$(4)-$(3)-$(1))
        target-powerset += $(if $(4),$(4)-$(2)-$($(2)-$(3)-id)-$(1))
        target-powerset += $(if $(4),$(4)-$(2)-$(3)-$(1))
        target-powerset += $(if $(4),$(4)-$($(2)-$(3)-id)-$(3)-$(1))
        target-powerset += $(if $(4),$(4)-$(2)-$($(2)-$(3)-id)-$(3)-$(1))
endif
