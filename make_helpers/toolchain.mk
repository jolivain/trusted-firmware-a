#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(dir $(lastword $(MAKEFILE_LIST)))build_env.mk

ifneq ($(or $(V),0),0)
        $(info ################################################################################)
        $(info # Toolchain configuration                                                      #)
        $(info ################################################################################)
endif

#
# Configure tool classes that we recognize.
#
# In the context of this build system, a tool class identifies a specific role
# or type of tool in the toolchain.
#

tool-classes := ar # Archivers
tool-classes += as # Assemblers
tool-classes += cc # C compilers
tool-classes += cpp # C preprocessors
tool-classes += ld # Linkers
tool-classes += oc # Object copiers
tool-classes += od # Object dumpers
tool-classes += dtc # Device tree compilers

tool-class-name-ar := Archiver
tool-class-name-as := Assembler
tool-class-name-cc := C compiler
tool-class-name-cpp := C preprocessor
tool-class-name-ld := Linker
tool-class-name-oc := Object copier
tool-class-name-od := Object dumper
tool-class-name-dtc := Device tree compiler

define check-tool-class-name
        $(eval tool-class := $(1))

        ifndef tool-class-name-$(tool-class)
                $$(error no name registered for tool class `$(tool-class)`)
        endif
endef

$(foreach tool-class,$(tool-classes), \
        $(eval $(call check-tool-class-name,$(tool-class))))

ifneq ($(or $(V),0),0)
        $(info )
        $(info Registered tool classes:)
        $(info )

        $(foreach tool-class,$(tool-classes), \
                $(info $(space) - $(tool-class-name-$(tool-class)) [$(tool-class)]))
endif

#
# Configure tools that we recognize.
#
# Here we declare the list of specific toolchain tools that we know how to
# interact with. We don't organize these into tool classes yet - that happens
# further down.
#

tools := arm-ar # armar (Arm Compiler for Embedded)
tools += arm-clang # armclang (Arm Compiler for Embedded)
tools += arm-link # armlink (Arm Compiler for Embedded)
tools += arm-fromelf # fromelf (Arm Compiler for Embedded)

tools += llvm-ar # llvm-ar (LLVM Project)
tools += llvm-clang # clang (LLVM Project)
tools += llvm-lld # lld (LLVM Project)
tools += llvm-objcopy # llvm-objcopy (LLVM Project)
tools += llvm-objdump # llvm-objdump (LLVM Project)

tools += gnu-ar # gcc-ar (GNU Compiler Collection & GNU Binary Utilities)
tools += gnu-gcc # gcc (GNU Compiler Collection)
tools += gnu-ld # ld (GNU Binary Utilities)
tools += gnu-objcopy # objcopy (GNU Binary Utilities)
tools += gnu-objdump # objdump (GNU Binary Utilities)

tools += dtc # Device Tree Compiler

tool-name-arm-ar := armar (Arm Compiler for Embedded)
tool-name-arm-clang := armclang (Arm Compiler for Embedded)
tool-name-arm-link := armlink (Arm Compiler for Embedded)
tool-name-arm-fromelf := fromelf (Arm Compiler for Embedded)

tool-name-llvm-ar := llvm-ar (LLVM Project)
tool-name-llvm-clang := clang (LLVM Project)
tool-name-llvm-lld := lld (LLVM Project)
tool-name-llvm-objcopy := llvm-objcopy (LLVM Project)
tool-name-llvm-objdump := llvm-objdump (LLVM Project)

tool-name-gnu-ar := gcc-ar (GNU Compiler Collection & GNU Binary Utilities)
tool-name-gnu-gcc := gcc (GNU Compiler Collection)
tool-name-gnu-ld := ld (GNU Binary Utilities)
tool-name-gnu-objcopy := objcopy (GNU Binary Utilities)
tool-name-gnu-objdump := objdump (GNU Binary Utilities)

tool-name-dtc := dtc (Device Tree Compiler)

define check-tool-name
        $(eval tool := $(1))

        ifndef tool-name-$(tool)
                $$(error no name registered for tool `$(tool)`)
        endif
endef

$(foreach tool,$(tools), \
        $(eval $(call check-tool-name,$(tool))))

#
# Assign tools to tool classes.
#
# Multifunctional tools, i.e. tools which can perform multiple roles in a
# toolchain, may be specified in multiple tool class lists. For example, a C
# compiler which can also perform the role of a linker may be placed in both
# `tools-cc` and `tools-ld`.
#

tools-ar := arm-ar llvm-ar gnu-ar # Archivers
tools-as := arm-clang llvm-clang gnu-gcc # Assemblers
tools-cc := arm-clang llvm-clang gnu-gcc # C compilers
tools-cpp := arm-clang llvm-clang gnu-gcc # C preprocessors
tools-ld := arm-clang arm-link llvm-clang llvm-lld gnu-gcc gnu-ld # Linkers
tools-oc := arm-fromelf llvm-objcopy gnu-objcopy # Object copiers
tools-od := arm-fromelf llvm-objdump gnu-objdump # Object dumpers
tools-dtc := dtc # Device tree compilers

define check-tool-class-tools
        $(eval tool-class := $(1))

        ifndef tools-$(tool-class)
                $$(error no tools registered to handle tool class `$(tool-class)`)
        endif
endef

$(foreach tool-class,$(tool-classes), \
        $(eval $(call check-tool-class-tools,$(tool-class))))

ifneq ($(or $(V),0),0)
        $(info )
        $(info Registered tools:)

        $(foreach tool-class,$(tool-classes), \
                $(info ) \
                $(info $(space) - $(tool-class-name-$(tool-class)) [$(tool-class)]:) \
                $(info ) \
                $(foreach tool,$(tools-$(tool-class)), \
                        $(info $(space)   - $(tool-name-$(tool)) [$(tool)])))
endif

#
# Identify toolchains.
#
# A toolchain, in the context of this build system, refers literally to a chain
# of tools used for one particular target.
#
# Tools in a toolchain may come from any combination of vendors. For example,
# when targeting an AArch64 platform you might choose to use Arm Clang as your C
# compiler and GCC as your linker, but we would consider these to be part of the
# same AArch64 toolchain.
#
# Toolchains are configured by the parent Makefile, which must define the
# following variables:
#
#   - `toolchains`: the list of toolchain identifiers to configure
#   - `<toolchain>-name`: the name of each toolchain
#
# The toolchain name is used in this section to dump human-readable information
# about how each toolchain is configured.
#

define check-toolchain-name
        $(eval toolchain := $(1))

        ifndef toolchain-name-$(toolchain)
                $$(error no name registered for toolchain `$(toolchain)`)
        endif
endef

$(foreach toolchain,$(toolchains), \
        $(eval $(call check-toolchain-name,$(toolchain))))

ifneq ($(or $(V),0),0)
        $(info )
        $(info Registered toolchains:)
        $(info ) \

        $(foreach toolchain,$(toolchains), \
                $(info $(space) - $(toolchain-name-$(toolchain)) [$(toolchain)]))
endif

#
# Default tools for each toolchain.
#
# Toolchains can specify a default path to any given tool with a tool class.
# These values are used in the absence of user-specified values, and are
# configured by the parent Makefile using variables of the form:
#
#   - <toolchain>-<tool-class>-default
#
# For example, the default C compiler for the AArch32 and AArch64 toolchains
# could be configured with:
#
#   - aarch32-cc-default
#   - aarch64-cc-default
#

define check-tool-class-default
        $(eval toolchain := $(1))
        $(eval tool-class := $(2))

        ifndef $(toolchain)-$(tool-class)-default
                $$(error no default value specified for tool class `$(tool-class)` of toolchain `$(toolchain)`)
        endif
endef

$(foreach toolchain,$(toolchains),$(foreach tool-class,$(tool-classes), \
        $(eval $(call check-tool-class-default,$(toolchain),$(tool-class)))))

ifneq ($(or $(V),0),0)
        $(info )
        $(info Registered toolchain defaults:)

        $(foreach toolchain,$(toolchains), \
                $(info ) \
                $(info $(space) - $(toolchain-name-$(toolchain)) [$(toolchain)]:) \
                $(info ) \
                $(foreach tool-class,$(tool-classes), \
                        $(info $(space)     - $(tool-class-name-$(tool-class)) [$(tool-class)]: $($(toolchain)-$(tool-class)-default))))
endif

#
# User-specified toolchain paths.
#
# Paths for the host toolchain tools can be explicitly specified by the user
# in whatever mechanism the parent Makefile prefers. For example, TF-A that uses
# the traditional implicit variables (e.g. `CC`, `CPP`, `LD`, etc.).
#

ifneq ($(or $(V),0),0)
        $(info )
        $(info User-specified toolchain values:)

        $(foreach toolchain,$(toolchains), \
                $(info ) \
                $(info $(space) - $(toolchain-name-$(toolchain)) [$(toolchain)]:) \
                $(info ) \
                $(foreach tool-class,$(tool-classes), \
                        $(info $(space)     - $(tool-class-name-$(tool-class)) [$(tool-class)]: $(or $($(toolchain)-$(tool-class)),<no user-specified value>))))
endif

#
# Helper functions to identify toolchain tools.
#
# The functions defined in this section tools return a tool identifier when
# given a path to a binary. We generally check a version or help string to give
# us a more reliable idea than looking at the path alone (e.g. `gcc` on macOS is
# actually Apple Clang).
#
# Each tool-guessing function (`guess-tool-<tool>`) takes a single argument
# giving the path to the tool to guess, and returns a non-empty value if the
# tool corresponds to the tool identifier `<tool>`:
#
#     $(call guess-tool-llvm-clang,aarch64-none-elf-gcc) # <empty>
#     $(call guess-tool-gnu-gcc,aarch64-none-elf-gcc) # <non-empty>
#
# The `guess-tool` function tries to find the corresponding tool identifier
# for a tool given its path. It takes two arguments:
#
#   - $(1): a list of candidate tool identifiers to check
#   - $(2): the path to the tool to identify
#
# If any of the guess functions corresponding to candidate tool identifiers
# return a non-empty value then the tool identifier of the first function to do
# so is returned:
#
#     $(call guess-tool,gnu-gcc llvm-clang,armclang) # <empty>
#     $(call guess-tool,gnu-gcc llvm-clang,clang-14) # llvm-clang
#     $(call guess-tool,gnu-gcc llvm-clang,aarch64-none-elf-gcc-12) # gnu-gcc
#
# Tools are checked in the order that they appear in `tools-<tool-class>`, and
# the first match is returned.
#

guess-tool-arm-ar = $(shell $(1) --version 2>&1 | grep -o "Tool: armar")
guess-tool-arm-clang = $(shell $(1) --version 2>&1 | grep -o "Tool: armclang")
guess-tool-arm-link = $(shell $(1) --help 2>&1 | grep -o "Tool: armlink")
guess-tool-arm-fromelf = $(shell $(1) --help 2>&1 | grep -o "Tool: fromelf")

guess-tool-llvm-ar = $(shell $(1) --help 2>&1 | grep -o "LLVM Archiver")
guess-tool-llvm-clang = $(shell $(1) -v 2>&1 | grep -o "clang version")
guess-tool-llvm-lld = $(shell $(1) --help 2>&1 | grep -o "OVERVIEW: lld")
guess-tool-llvm-objcopy = $(shell $(1) --help 2>&1 | grep -o "llvm-objcopy tool")
guess-tool-llvm-objdump = $(shell $(1) --help 2>&1 | grep -o "llvm object file dumper")

guess-tool-gnu-ar = $(shell $(1) --version 2>&1 | grep -o "GNU ar")
guess-tool-gnu-gcc = $(shell $(1) -v 2>&1 | grep -o "gcc version")
guess-tool-gnu-ld = $(shell $(1) -v 2>&1 | grep -o "GNU ld")
guess-tool-gnu-objcopy = $(shell $(1) --version 2>&1 | grep -o "GNU objcopy")
guess-tool-gnu-objdump = $(shell $(1) --version 2>&1 | grep -o "GNU objdump")

guess-tool-dtc = $(shell $(1) --version 2>&1 | grep -o "Version: DTC")

guess-tool = $(firstword $(foreach candidate,$(1), \
        $(if $(call guess-tool-$(candidate),$(2)),$(candidate))))

#
# Locate and identify tools belonging to each toolchain.
#
# Because many C compilers offer facilities for locating other tools in the
# toolchain, we can fall back to guessing their location if the user hasn't
# specified them.
#
# In some toolchains the C compiler can also perform as other tools in the
# toolchain, and where that's the case we will default to using the C compiler
# for these tools if the user hasn't explicitly overridden them.
#

guess-arm-clang-ar = # Fall back to the GNU archiver
guess-arm-clang-as = $(1)
guess-arm-clang-cpp = $(1)
guess-arm-clang-ld = # Fall back to the GNU linker
guess-arm-clang-oc = # Fall back to the GNU object copier
guess-arm-clang-od = # Fall back to the GNU object dumper

guess-llvm-clang-ar = $(shell $(1) --print-prog-name llvm-ar 2>$(nul))
guess-llvm-clang-as = $(1)
guess-llvm-clang-cpp = $(1)
guess-llvm-clang-ld = $(shell $(1) --print-prog-name ld.lld 2>$(nul))
guess-llvm-clang-oc = $(shell $(1) --print-prog-name llvm-objcopy 2>$(nul))
guess-llvm-clang-od = $(shell $(1) --print-prog-name llvm-objdump 2>$(nul))

guess-gnu-gcc-ar = $(patsubst %$(notdir $(1)),%$(subst gcc,gcc-ar,$(notdir $(1))),$(1))
guess-gnu-gcc-as = $(1)
guess-gnu-gcc-cpp = $(1)
guess-gnu-gcc-ld = $(if $(filter 1,$(ENABLE_LTO)),$(1),$(shell $(1) --print-prog-name ld.bfd 2>$(nul)))
guess-gnu-gcc-oc = $(shell $(1) --print-prog-name objcopy 2>$(nul))
guess-gnu-gcc-od = $(shell $(1) --print-prog-name objdump 2>$(nul))

define locate-toolchain-cc
        $(eval toolchain := $(1))

        $(toolchain)-cc := $$(strip \
                $$(or $$($(toolchain)-cc),$$($(toolchain)-cc-default)))
        $(toolchain)-cc-id := $$(strip \
                $$(call guess-tool,$$(tools-cc),$$($(toolchain)-cc)))
endef

define locate-toolchain-tool
        $(eval toolchain := $(1))
        $(eval tool-class := $(2))

        ifndef $(toolchain)-$(tool-class)
                $(toolchain)-$(tool-class) := $$(strip \
                        $$(call guess-$$($(toolchain)-cc-id)-$(tool-class),$$($(toolchain)-cc)))

                ifeq ($$($(toolchain)-$(tool-class)),)
                        $(toolchain)-$(tool-class) := $$(strip \
                                $$($(toolchain)-$(tool-class)-default))
                endif
        endif

        $(toolchain)-$(tool-class)-id := $$(strip \
                $$(call guess-tool,$$(tools-$(tool-class)),$$($$(toolchain)-$(tool-class))))
endef

define canonicalize-toolchain-tool-path
        $(eval toolchain := $(1))
        $(eval tool-class := $(2))

        $(toolchain)-$(tool-class) := $$(strip $$(or \
                $$(call which,$$($(toolchain)-$(tool-class))), \
                $$($(toolchain)-$(tool-class))))
endef

define locate-toolchain
        $(eval toolchain := $(1))

        $$(eval $$(call locate-toolchain-cc,$(toolchain)))
        $$(eval $$(call canonicalize-toolchain-tool-path,$(toolchain),cc))

        $$(foreach tool-class,$$(filter-out cc,$$(tool-classes)), \
                $$(eval $$(call locate-toolchain-tool,$(toolchain),$$(tool-class))) \
                $$(eval $$(call canonicalize-toolchain-tool-path,$(toolchain),$$(tool-class))))
endef

$(foreach toolchain,$(toolchains), \
        $(eval $(call locate-toolchain,$(toolchain))))

ifneq ($(or $(V),0),0)
        $(info )
        $(info Toolchain configuration:)

        $(foreach toolchain,$(toolchains), \
                $(info ) \
                $(info $(space) - $(toolchain-name-$(toolchain)) [$(toolchain)]:) \
                $(info ) \
                $(foreach tool-class,$(tool-classes), \
                        $(info $(space)     - $(tool-class-name-$(tool-class)) [$(tool-class)]: $($(toolchain)-$(tool-class)) [$($(toolchain)-$(tool-class)-id)])))
endif
