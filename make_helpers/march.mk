#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This Build helper file is used to determine a suitable march build
# option to be used, based on platform provided ARM_ARCH_MAJOR/MINOR
# data and compiler supported march version.

# Set the compiler's target architecture profile based on
# ARM_ARCH_MAJOR ARM_ARCH_MINOR options

# This function is to find if a given march options is supported by compiler.
# if the march option is supported by compiler if the option is supported
# will return the same march option with provided value '-march=xxxxx'
define march_option
    $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null >/dev/null 2>&1; then echo $(1); fi)
endef

# This function is used to find the highest march value supported by the given compiler.
# we try to print targets supported and find armvX.X-a or armvX-a values and from that list,
# return the highest supported arch value.
#
# Example on a gcc-12.2 arm64 toolchain this will return armv9-a
# [...]
#./aarch64-none-elf-gcc --target-help -march=foo
# cc1: error: unknown value 'foo' for '-march'
# cc1: note: valid arguments are: armv8-a armv8.1-a armv8.2-a armv8.3-a armv8.4-a armv8.5-a
# armv8.6-a armv8.7-a armv8.8-a armv8-r armv9-a
# [...]
#
# We expect from Platform to provide a correct Major/Minor value but expecting something
# from compiler with unsupported march means we shouldnt fail without trying anything,
# so here we try to find highest supported arch value and use that for compilation.
#
define cc_march
$(shell if $(CC) --target-help -march=foo 2>&1 | grep -o -P 'armv...-a|armv.-a' | tail -1; then echo $(1); fi)
endef

ifdef MARCH_DIRECTIVE
march-directive		:= $(MARCH_DIRECTIVE)
else

ifeq (${ARM_ARCH_MINOR},0)
provided-march = armv${ARM_ARCH_MAJOR}-a
else
provided-march = armv${ARM_ARCH_MAJOR}.${ARM_ARCH_MINOR}-a
endif

ifeq ($(findstring clang,$(notdir $(CC))),)
# If we dont support current march version from gcc compiler, try with lower arch based on
# availability. In TF-A there is no hard rule for need of higher version march for basic
# functionality, denying a build on only this fact doesn't look correct, so try with lower
# march values from whats available from compiler.
ifeq (,$(findstring $(provided-march), $(call march_option, -march=${provided-march})))
available-march	:= $(call cc_march)
provided-march := ${available-march}
endif
endif

march-directive := -march=${provided-march}
endif
