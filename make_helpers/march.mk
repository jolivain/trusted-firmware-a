#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This build helper makefile is used to determine a suitable march build
# option to be used, based on platform provided ARM_ARCH_MAJOR/MINOR
# data and compiler supported march version.

# Set the compiler's target architecture profile based on
# ARM_ARCH_MAJOR and ARM_ARCH_MINOR options.

# This function is used to find if a given march option is supported by the compiler.
# if the march option is supported by compiler it will return the same march option
# with provided value '-march=xxxxx'
define march_option
    $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null >/dev/null 2>&1; then echo $(1); fi)
endef

# This function is used to find the best march value supported by the given compiler.
# we try to print targets supported and find armvX.X-a or armvX-a values and from that list,
# return the best supported arch value based on arch_major from platform.
#
# Example on a gcc-12.2 arm64 toolchain this will return armv9-a if platform requested for armv9.2-a
# [...]
#./aarch64-none-elf-gcc --target-help -march=foo
# cc1: error: unknown value 'foo' for '-march'
# cc1: note: valid arguments are: armv8-a armv8.1-a armv8.2-a armv8.3-a armv8.4-a armv8.5-a
# armv8.6-a armv8.7-a armv8.8-a armv8-r armv9-a
# [...]
#
# Similarly on gcc 11.3 it would return armv8.6-a if platform requested armv8.8-a
#
# Expected input to this function is '-march=xxxxx'
define major_best_avail_march
$(shell if $(CC) --target-help -march=foo 2>&1 | grep -o -P 'armv...-a|armv.-a' | grep armv$(ARM_ARCH_MAJOR) | tail -1; then echo $(1); fi)
endef

# This function is used to just return latest march value supported by the given compiler.
#
# Example:
# this would return armv8.6-a on a gcc-11.3 when platform requested for armv9.0-a
# on gcc-12.2 it would return armv9.0-a when platform requested for armv8.9-a
#
# Thus this function should be used in conjunction with best_avail_march, when best match
# is not found it should be ok to try with lastest known supported march value from the
# compiler.
#
# Expected input to this function is '-march=xxxxx'
define lastest_match_march
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

# We expect from Platform to provide a correct Major/Minor value but expecting something
# from compiler with unsupported march means we shouldn't fail without trying anything,
# so here we try to find best supported march value and use that for compilation.
# If we don't support current march version from gcc compiler, try with lower arch based on
# availability. In TF-A there is no hard rule for need of higher version march for basic
# functionality, denying a build on only this fact doesn't look correct, so try with best
# or latest march values from whats available from compiler.
ifeq (,$(findstring $(provided-march), $(call march_option, -march=${provided-march})))
    available-march	:= $(call major_best_avail_march)

ifeq (, $(available-march))
    available-march	:= $(call lastest_match_march)
endif

# If we fail to come up with any march value so far, don't update
# provided-march and thus allow the build to fail using the provided-march
# derived based on arch_major and arch_minor values.
ifneq (,$(available-march))
    provided-march := ${available-march}
endif

endif # provided-march supported
endif # not clang

march-directive := -march=${provided-march}
endif # MARCH_DIRECTIVE
