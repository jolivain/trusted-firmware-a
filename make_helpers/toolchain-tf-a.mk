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
host-ar-flags += $(HOSTARFLAGS)

host-as := $(HOSTAS)
host-as-flags += $(HOSTASFLAGS)

host-cc := $(HOSTCC)
host-cc-flags += $(HOSTCCFLAGS)

host-cpp := $(HOSTCPP)
host-cpp-flags += $(HOSTCPPFLAGS)

host-ld := $(HOSTLD)
host-ld-flags += $(HOSTLDFLAGS)

host-oc := $(HOSTOC)
host-oc-flags += $(HOSTOCFLAGS)

host-od := $(HOSTOD)
host-od-flags += $(HOSTODFLAGS)

host-dtc := $(HOSTDTC)
host-dtc-flags += $(HOSTDTCFLAGS)

$(ARCH)-ar := $(if $(filter-out default,$(origin AR)),$(AR))
$(ARCH)-ar-flags += $(ARFLAGS)

$(ARCH)-as := $(if $(filter-out default,$(origin AS)),$(AS))
$(ARCH)-as-flags += $(ASFLAGS)

$(ARCH)-cc := $(if $(filter-out default,$(origin CC)),$(CC))
$(ARCH)-cc-flags += $(CCFLAGS)

$(ARCH)-cpp := $(if $(filter-out default,$(origin CPP)),$(CPP))
$(ARCH)-cpp-flags += $(CPPFLAGS)

$(ARCH)-ld := $(if $(filter-out default,$(origin LD)),$(LD))
$(ARCH)-ld-flags += $(LDFLAGS)

$(ARCH)-oc := $(if $(filter-out default,$(origin OC)),$(OC))
$(ARCH)-oc-flags += $(OCFLAGS)

$(ARCH)-od := $(if $(filter-out default,$(origin OD)),$(OD))
$(ARCH)-od-flags += $(ODFLAGS)

$(ARCH)-dtc := $(if $(filter-out default,$(origin DTC)),$(DTC))
$(ARCH)-dtc-flags += $(DTCFLAGS)

include $(MAKE_HELPERS_DIRECTORY)toolchain-host.mk
include $(MAKE_HELPERS_DIRECTORY)toolchain-$(ARCH).mk
