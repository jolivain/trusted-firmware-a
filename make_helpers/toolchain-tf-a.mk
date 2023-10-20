#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# TF-A uses three toolchains:
#
#   - The host toolchain (`host`) for building native tools
#   - The AArch32 toolchain (`aarch32`) for building Arm AArch32 images
#   - The AArch64 toolchain (`aarch64`) for building Arm AArch64 images
#
# Only one of the two Arm toolchains is enabled in any given build.
#

host-ar := $(HOSTAR)
host-as := $(HOSTAS)
host-cc := $(HOSTCC)
host-cpp := $(HOSTCPP)
host-ld := $(HOSTLD)
host-oc := $(HOSTOC)
host-od := $(HOSTOD)
host-dtc := $(HOSTDTC)

$(ARCH)-ar := $(if $(filter-out default,$(origin AR)),$(AR))
$(ARCH)-as := $(if $(filter-out default,$(origin AS)),$(AS))
$(ARCH)-cc := $(if $(filter-out default,$(origin CC)),$(CC))
$(ARCH)-cpp := $(if $(filter-out default,$(origin CPP)),$(CPP))
$(ARCH)-ld := $(if $(filter-out default,$(origin LD)),$(LD))
$(ARCH)-oc := $(if $(filter-out default,$(origin OC)),$(OC))
$(ARCH)-od := $(if $(filter-out default,$(origin OD)),$(OD))
$(ARCH)-dtc := $(if $(filter-out default,$(origin DTC)),$(DTC))

include $(MAKE_HELPERS_DIRECTORY)toolchain-host.mk
include $(MAKE_HELPERS_DIRECTORY)toolchain-$(ARCH).mk
