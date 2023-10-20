#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(dir $(lastword $(MAKEFILE_LIST)))build_env.mk
include $(dir $(lastword $(MAKEFILE_LIST)))utilities.mk

ifneq ($(or $(V),0),0)
        $(info ################################################################################)
        $(info # Toolchain configuration                                                      #)
        $(info ################################################################################)
endif

#
# Configure tool classes that we recognize.
#
# In the context of this build system, a tool class identifies a specific role
# or element in the toolchain:
#
# - Archivers (ar): archivers take individual files and package them into a
#     single archive file.
# - Assemblers (as): convert assembly language files into machine code object
#     files.
# - C compilers (cc): turn C source code files into object files or executables.
# - C preprocessors (cpp): modify and expand C source code before it is
#     compiled.
# - Linkers (ld): join multiple object files into a single executable or
#     library.
# - Object copiers (oc): reformat object files into different object file
#     formats.
# - Object dumpers (od): extract and display information from object files or
#     executables.
# - Device tree compilers (dtc): translate device tree source files into binary
#     device tree blobs.
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
# interact with. We don't organize these into tool classes yet (that happens
# further down).
#

tools := arm-ar # armar (Arm Compiler for Embedded)
tools += arm-clang # armclang (Arm Compiler for Embedded)
tools += arm-fromelf # fromelf (Arm Compiler for Embedded)

tools += llvm-ar # llvm-ar (LLVM Project)
tools += llvm-clang # clang (LLVM Project)
tools += llvm-objcopy # llvm-objcopy (LLVM Project)
tools += llvm-objdump # llvm-objdump (LLVM Project)

tools += gnu-ar # gcc-ar (GNU Compiler Collection & GNU Binary Utilities)
tools += gnu-gcc # gcc (GNU Compiler Collection)
tools += gnu-objcopy # objcopy (GNU Binary Utilities)
tools += gnu-objdump # objdump (GNU Binary Utilities)

tools += dtc # Device Tree Compiler

tool-name-arm-ar := armar (Arm Compiler for Embedded)
tool-name-arm-clang := armclang (Arm Compiler for Embedded)
tool-name-arm-fromelf := fromelf (Arm Compiler for Embedded)

tool-name-llvm-ar := llvm-ar (LLVM Project)
tool-name-llvm-clang := clang (LLVM Project)
tool-name-llvm-objcopy := llvm-objcopy (LLVM Project)
tool-name-llvm-objdump := llvm-objdump (LLVM Project)

tool-name-gnu-ar := gcc-ar (GNU Compiler Collection & GNU Binary Utilities)
tool-name-gnu-gcc := gcc (GNU Compiler Collection)
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
tools-ld := arm-clang llvm-clang gnu-gcc # Linkers
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
guess-tool-arm-fromelf = $(shell $(1) --help 2>&1 | grep -o "Tool: fromelf")

guess-tool-llvm-ar = $(shell $(1) --help 2>&1 | grep -o "LLVM Archiver")
guess-tool-llvm-clang = $(shell $(1) -v 2>&1 | grep -o "clang version")
guess-tool-llvm-objcopy = $(shell $(1) --help 2>&1 | grep -o "llvm-objcopy tool")
guess-tool-llvm-objdump = $(shell $(1) --help 2>&1 | grep -o "llvm object file dumper")

guess-tool-gnu-ar = $(shell $(1) --version 2>&1 | grep -o "GNU ar")
guess-tool-gnu-gcc = $(shell $(1) -v 2>&1 | grep -o "gcc version")
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

guess-arm-clang-ar = $(shell $(1) --print-prog-name armar 2>$(nul))
guess-arm-clang-as = $(1)
guess-arm-clang-cpp = $(1)
guess-arm-clang-ld = $(1)
guess-arm-clang-oc = $(shell $(1) --print-prog-name fromelf 2>$(nul))
guess-arm-clang-od = $(shell $(1) --print-prog-name fromelf 2>$(nul))

guess-llvm-clang-ar = $(shell $(1) --print-prog-name llvm-ar 2>$(nul))
guess-llvm-clang-as = $(1)
guess-llvm-clang-cpp = $(1)
guess-llvm-clang-ld = $(1)
guess-llvm-clang-oc = $(shell $(1) --print-prog-name llvm-objcopy 2>$(nul))
guess-llvm-clang-od = $(shell $(1) --print-prog-name llvm-objdump 2>$(nul))

guess-gnu-gcc-ar = $(patsubst %$(notdir $(1)),%$(subst gcc,gcc-ar,$(notdir $(1))),$(1))
guess-gnu-gcc-as = $(1)
guess-gnu-gcc-cpp = $(1)
guess-gnu-gcc-ld = $(1)
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
                $$(realpath $$(call which,$$($(toolchain)-$(tool-class)))), \
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

#
# Helper functions to retrieve parameter(s) for tool arguments.
#
# Some command line arguments require parametrisation. For example, the `-mcpu`
# argument to GCC takes a parameter giving the identifier of the CPU to target.
# This function defines a common mechanism for passing additional information to
# these sorts of arguments.
#
# This function takes the following arguments:
#
#   - $(1): parameter namespace
#   - $(2): toolchain identifier (host, aarch32, aarch64)
#   - $(3): tool class (e.g. cc, ld, od, etc.)
#   - $(4): argument identifier
#
# It expands to values of the following variables, in the given order:
#
#   - <namespace>-<toolchain>-<tool>-<tool-class>-<argument>
#   - <namespace>-<toolchain>-<tool>-<argument>
#   - <namespace>-<toolchain>-<tool-class>-<argument>
#   - <namespace>-<toolchain>-<argument>
#   - <namespace>-<tool>-<tool-class>-<argument>
#   - <namespace>-<tool>-<argument>
#   - <namespace>-<tool-class>-<argument>
#   - <namespace>-<argument>
#   - <toolchain>-<tool>-<tool-class>-<argument>
#   - <toolchain>-<tool>-<argument>
#   - <toolchain>-<tool-class>-<argument>
#   - <toolchain>-<argument>
#   - <tool>-<tool-class>-<argument>
#   - <tool>-<argument>
#   - <tool-class>-<argument>
#   - <argument>
#
# The values are returned in descending order of specifity. If you need this
# list in ascending order then simply reverse it, and if you only need the most
# or least specific value then you can use `$(firstword)` and `$(lastword)`
# respectively.
#
# For example, if you wanted to know all of the input files that would be passed
# to GCC when compiling C source files and targeting AArch64 in the `xyz`
# namespace, you would use the following:
#
#     $(call tool-parameters,xyz,aarch64,cc,input-files)
#
# This would return the values of the following variables:
#
#   - xyz-aarch64-gnu-gcc-cc-input-files
#   - xyz-aarch64-gnu-gcc-input-files
#   - xyz-aarch64-cc-input-files
#   - xyz-aarch64-input-files
#   - xyz-gnu-gcc-cc-input-files
#   - xyz-gnu-gcc-input-files
#   - xyz-cc-input-files
#   - xyz-input-files
#   - aarch64-gnu-gcc-cc-input-files
#   - aarch64-gnu-gcc-input-files
#   - aarch64-cc-input-files
#   - aarch64-input-files
#   - gnu-gcc-cc-input-files
#   - gnu-gcc-input-files
#   - cc-input-files
#   - input-files
#
# This mechanism allows parameters to be modified arbitrarily from wherever is
# most convenient.
#

tool-parameter = $(firstword $(call tool-parameters,$(1),$(2),$(3),$(4)))

tool-parameters = $(strip $(call tool-parameters1,$(call powerset,$(1) $(2) $($(2)-$(3)-id) $(3),-),$(4)))
tool-parameters1 = $(foreach v,$(addsuffix -$(2),$(1)),$($(v))) $($(2))

#
# Helper functions to generate command lines for each tool.
#
# This section defines a set of functions (`cc`, `cpp`, `ld`, etc.) which
# generate command lines for each tool given.
#
# Command line functions take three arguments:
#
#   - $(1): namespace
#   - $(2): toolchain identifier (e.g. host, aarch32, aarch64)
#   - $(3): additional argument identifiers (e.g. lto)
#
# The namespace is an arbitrary value used to mitigate against
# cross-contamination of parameters between multiple invocations of the
# function, and should generally be a unique value directly correlated with the
# output(s), e.g.
#
#     lib1-defines := LIB1=1
#     lib1-input-files := src/lib.c
#     lib1-output-file := lib1/lib.o
#
#     lib2-defines := LIB2=1
#     lib2-input-files := src/lib.c
#     lib2-output-file := lib2/lib.o
#
#     $(call cc,lib1,host)
#     $(call cc,lib2,host)
#
# When calling a command line function, argument identifiers specify additional
# arguments to be passed to the tool in a way that abstracts toolchain-specific
# details. Each argument identifier corresponds to a set of functions that
# generate the toolchain-specific arguments, in the form:
#
#     <tool>-<argument>
#
# For example:
#
#     xyz-input-files := hello.c
#     xyz-output-file := hello.o
#
#     llvm-clang-a = --ape # Defines the `a` argument for LLVM Clang
#     llvm-clang-b = --bee # Defines the `b` argument for LLVM Clang
#     llvm-clang-c = --cod # Defines the `c` argument for LLVM Clang
#
#     gnu-gcc-a = --ant # Defines the `a` argument for GNU GCC
#     gnu-gcc-b = --bat # Defines the `b` argument for GNU GCC
#     gnu-gcc-c = --cat # Defines the `c` argument for GNU GCC
#
#     $(info $(call cc,xyz,host,a b c))
#     #                         ^ ^ ^
#
#     # `CC` is GNU GCC: gcc -c hello.c -o hello.o --ant --bat --cat
#     # `CC` is LLVM Clang: clang -c hello.c -o hello.o --ape --bee --cod
#
# These argument functions are given the following arguments:
#
#   - $(1): parameter namespace
#   - $(2): toolchain identifier (host, aarch32, aarch64)
#   - $(3): tool class (e.g. cc, ld, ar)
#

proxy-tool = $(strip $($(2)-$(3)) $(call proxy-tool1,$(1),$(2),$(3),$(4)))
proxy-tool1 = $(call proxy-tool2,$(1),$(call proxy-tool-response-argument,$(1),$(2),$(3)),$(call proxy-tool-arguments,$(1),$(2),$(3),$(4)))
proxy-tool2 = $(if $(2),$(2)$(file >$($(1)-response-file),$(3)),$(3))

proxy-tool-response-argument = $(if $($(1)-response-file),$(call proxy-tool-argument,$(1),$(2),$(3),response-file))

proxy-tool-arguments = $(strip $(foreach a,$(call tool-parameters,$(1),$(2),$(3),arguments) $(4),$(call proxy-tool-argument,$(1),$(2),$(3),$(a))))
proxy-tool-argument = $(call argument-$(4)-$($(2)-$(3)-id),$(1),$(2),$(3))

define define-tool-helpers
        $(eval tool-class := $(1))

        $(tool-class) = $$(call proxy-tool,$$(1),$$(2),$(tool-class),$$(3))
        $(tool-class)-args = $$(call proxy-tool-arguments,$$(1),$$(2),$(tool-class),$$(3))
endef

$(foreach tool-class,$(tool-classes), \
        $(eval $(call define-tool-helpers,$(tool-class))))

include $(MAKE_HELPERS_DIRECTORY)toolchain-arguments.mk
