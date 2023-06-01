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

ifeq (${ARM_ARCH_MINOR},0)
provided-march = armv${ARM_ARCH_MAJOR}-a
else
provided-march = armv${ARM_ARCH_MAJOR}.${ARM_ARCH_MINOR}-a
endif

define march_option
    $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null >/dev/null 2>&1; then echo $(1); fi)
endef

define cc_march
$(shell if $(CC) --target-help -march=foo 2>&1 | grep -o -P 'armv...-a|armv.-a' | tail -1; then echo $(1); fi)
endef

# if we dont support current march version from compiler, try with lower
# arch based on availability. In TF-A there is no hard rule for need
# of higher version march for basic functionality, denying a build on only
# this fact doesnt look correct, so try with lower march values from whats
# available from compiler.
ifeq (,$(findstring $(provided-march), $(call march_option, -march=${provided-march})))
available-march	:= $(call cc_march)
$(warning ${provided-march} is not available with $(CC) trying now with ${available-march})
provided-march := ${available-march}
endif

march-directive := -march=${provided-march}