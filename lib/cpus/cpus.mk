#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Set the CPUs enabled by the platform.
#
# This is a list of CPUs that this build of TF-A must support, following the
# naming scheme used by the CPU support source files in `lib/cpus/$(ARCH)`.
#

CPUS_ENABLE := $(sort $(CPUS_ENABLE))

#
# Enable the CPU Operations component.
#
# This component includes CPU operations, miscellaneous helpers and errata
# workarounds for the CPUs configured for the platform.
#
# Without this option, the CPU Support library only exposes a minimal set of
# CPU-specific preprocessor definitions.
#

CPUS_ENABLE_CPU_OPERATIONS ?= 0

#
# Enable the CPU AMU component.
#
# Some Arm CPUs pre-dating FEAT_AMUv1 include support for an implementation-
# defined AMU. Images which would usually configure the architectural AMU should
# also enable this build option to ensure that these AMU implementations are
# configured instead.
#

CPUS_ENABLE_CPU_AMU ?= 0

#
# Enable the Errata Report component.
#
# The errata report is generated on boot by certain images, and details the set
# of errata that the platform suffers from, and whether workarounds have or have
# not been applied.
#

CPUS_ENABLE_ERRATA_REPORT ?= 0

#
# Enable the workaround for CVE-2017-5715.
#

CPUS_ENABLE_CVE_2017_5715_WORKAROUND ?= 0

#
# Basic sanity checks.
#

ifeq ($(CPUS_ENABLE),)
        $(error this platform has not configured any CPUs (`CPUS_ENABLE` is empty))
endif

ifneq ($(CPUS_ENABLE_CPU_AMU),0)
        ifeq ($(ENABLE_FEAT_AMU),0)
                $(error the CPU AMU component requires that AMU support be enabled (`CPUS_ENABLE_CPU_AMU=$(CPUS_ENABLE_CPU_AMU)` requires `ENABLE_FEAT_AMU=1`, have `ENABLE_FEAT_AMU=0`))
        endif
endif

ifneq ($(CPUS_ENABLE_CVE_2017_5715_WORKAROUND),0)
        ifeq ($(WORKAROUND_CVE_2017_5715),0)
                $(error this image requires the CVE-2017-5715 CPU workaround, but the workaround is not enabled (`CPUS_ENABLE_CVE_2017_5715_WORKAROUND=$(CPUS_ENABLE_CVE_2017_5715_WORKAROUND)` requires `WORKAROUND_CVE_2017_5715=1`, have `WORKAROUND_CVE_2017_5715=0`))
        endif

        ifneq ($(ARCH),aarch64)
                $(error the workaround for CVE-2017-5715 is only applicable in the AArch64 execution state (`CPUS_ENABLE_CVE_2017_5715_WORKAROUND=$(CPUS_ENABLE_CVE_2017_5715_WORKAROUND)` requires `ARCH=aarch64`, have `ARCH=$(ARCH)`))
        endif
endif

#
# Hardware-assisted coherency sanity checks.
#
# Some CPUs with support for hardware-assisted coherency do not support
# integration into platforms without hardware-assisted coherency - catch this
# incompatibility.
#

CPUS_NEED_HW_ASSISTED_COHERENCY := cortex_a55
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a65
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a65ae
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a75
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a76
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a76ae
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a77
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a78
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a78_ae
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a78c
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a510
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a710
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_a715
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_x1
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_x2
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_x3
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_hayes
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_hunter
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_hunter_elp_arm
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_chaberton
CPUS_NEED_HW_ASSISTED_COHERENCY += cortex_blackhawk
CPUS_NEED_HW_ASSISTED_COHERENCY += neoverse_e1
CPUS_NEED_HW_ASSISTED_COHERENCY += neoverse_n1
CPUS_NEED_HW_ASSISTED_COHERENCY += neoverse_n2
CPUS_NEED_HW_ASSISTED_COHERENCY += neoverse_v1
CPUS_NEED_HW_ASSISTED_COHERENCY += neoverse_v2
CPUS_NEED_HW_ASSISTED_COHERENCY += neoverse_poseidon
CPUS_NEED_HW_ASSISTED_COHERENCY += rainier

CPUS_ENABLED_NEED_HW_ASSISTED_COHERENCY := \
        $(filter $(CPUS_NEED_HW_ASSISTED_COHERENCY),$(CPUS_ENABLE))

ifneq ($(CPUS_ENABLED_NEED_HW_ASSISTED_COHERENCY),)
        ifeq ($(HW_ASSISTED_COHERENCY),0)
                $(error hardware-assisted coherency support must be enabled (have `HW_ASSISTED_COHERENCY=0`, need `HW_ASSISTED_COHERENCY=1`) to support these CPUs: $(CPUS_ENABLED_NEED_HW_ASSISTED_COHERENCY))
        endif
endif

#
# AArch32 execution state sanity checks.
#
# It does not make sense to include AArch32 registers in context switches when
# the platform includes non-AArch32 cores - catch this incompatibility.
#

CPUS_WITH_AARCH32_EXECUTION_STATE := aem_generic
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a35
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a53
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a55
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a57
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a72
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a73
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a75
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a78
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a78_ae
CPUS_WITH_AARCH32_EXECUTION_STATE += cortex_a78c
CPUS_WITH_AARCH32_EXECUTION_STATE += denver
CPUS_WITH_AARCH32_EXECUTION_STATE += generic

CPUS_ENABLED_WITHOUT_AARCH32_EXECUTION_STATE := \
        $(filter-out $(CPUS_WITH_AARCH32_EXECUTION_STATE),$(CPUS_ENABLE))

ifeq ($(ARCH),aarch64)
        ifneq ($(CTX_INCLUDE_AARCH32_REGS),0)
                ifneq ($(CPUS_ENABLED_WITHOUT_AARCH32_EXECUTION_STATE),)
                        $(error there are AArch64 CPUs enabled (via `CPUS_ENABLE`) which do not support the AArch32 execution state (required by `CTX_INCLUDE_AARCH32_REGS=$(CTX_INCLUDE_AARCH32_REGS)`): $(CPUS_ENABLED_WITHOUT_AARCH32_EXECUTION_STATE))
                endif
        endif
endif

#
# Configure the CPU AMU component.
#
# If one or more CPUs included in the system have an IMPDEF CPU AMU (i.e. not an
# architectural one), then we include the CPU AMU component if it has been
# enabled by the library consumer.
#

CPUS_HAVE_CPU_AMU := 0

CPUS_WITH_CPU_AMU := cortex_a75
CPUS_WITH_CPU_AMU += neoverse_n1

CPUS_ENABLED_WITH_CPU_AMU := \
        $(filter $(CPUS_WITH_CPU_AMU),$(CPUS_ENABLE))

ifneq ($(CPUS_ENABLE_CPU_AMU),0)
        ifneq ($(CPUS_ENABLED_WITH_CPU_AMU),)
                CPUS_HAVE_CPU_AMU := 1
        endif
endif

#
# Configure the DSU helpers component.
#
# This is an internal component of the CPU support library containing errata
# workarounds for the DynamIQ Shared Unit. It is only included into the CPU
# support library as a dependency of one or more of the enabled cores.
#

CPUS_HAVE_DSU_HELPERS := 0

CPUS_WITH_DSU_SUPPORT := cortex_a55
CPUS_WITH_DSU_SUPPORT += cortex_a65
CPUS_WITH_DSU_SUPPORT += cortex_a65ae
CPUS_WITH_DSU_SUPPORT += cortex_a75
CPUS_WITH_DSU_SUPPORT += cortex_a76
CPUS_WITH_DSU_SUPPORT += cortex_a510
CPUS_WITH_DSU_SUPPORT += cortex_a710
CPUS_WITH_DSU_SUPPORT += cortex_x2
CPUS_WITH_DSU_SUPPORT += neoverse_n1
CPUS_WITH_DSU_SUPPORT += neoverse_n2
CPUS_WITH_DSU_SUPPORT += neoverse_e1

CPUS_ENABLED_WITH_DSU_SUPPORT := \
        $(filter $(CPUS_WITH_DSU_SUPPORT),$(CPUS_ENABLE))

ifneq ($(CPUS_ENABLED_WITH_DSU_SUPPORT),)
        CPUS_HAVE_DSU_HELPERS := 1
endif

#
# Set up the library.
#

CPUS_DEFINES :=
CPUS_SOURCES :=

CPUS_INCLUDE_DIRS += include/lib/cpus
CPUS_INCLUDE_DIRS += include/lib/cpus/$(ARCH)

#
# Set up the library's CPU presence preprocessor definitions.
#
# For every CPU included in `CPUS_ENABLE`, a preprocessor definition of the
# form `HAVE_CPU_<XYZ>` is created, where `XYZ` is the upper-case name of the
# CPU. For example, given:
#
#     CPUS_ENABLE := cortex_a53 cortex_a57
#
# This would generate the preprocessor definitions:
#
#   - HAVE_CPU_CORTEX_A53=1
#   - HAVE_CPU_CORTEX_A57=1
#

CPUS_DEFINES += $(patsubst %,HAVE_CPU_%=1,$(call uppercase,$(CPUS_ENABLE)))

ifneq ($(filter neoverse_n%,$(CPUS_ENABLE)),)
        CPUS_DEFINES += HAVE_CPU_NEOVERSE_Nx=1
endif

#
# Set up the CPU Operations component.
#

ifneq ($(CPUS_ENABLE_CPU_OPERATIONS),0)
        CPUS_DEFINES += HAVE_CPU_OPERATIONS=1

        CPUS_SOURCES += lib/cpus/$(ARCH)/cpu_helpers.S
        CPUS_SOURCES += $(patsubst %,lib/cpus/$(ARCH)/%.S,$(CPUS_ENABLE))

        ifneq ($(CPUS_HAVE_DSU_HELPERS),0)
                CPUS_SOURCES += lib/cpus/$(ARCH)/dsu_helpers.S
        endif
endif

#
# Set up the CPU AMU component.
#

ifneq ($(CPUS_HAVE_CPU_AMU),0)
        CPUS_DEFINES += HAVE_CPU_AMU=1

        CPUS_SOURCES += lib/cpus/$(ARCH)/cpuamu.c
        CPUS_SOURCES += lib/cpus/$(ARCH)/cpuamu_helpers.S
        CPUS_SOURCES += $(patsubst %,lib/cpus/$(ARCH)/%_pubsub.c,$(CPUS_ENABLED_WITH_CPU_AMU))
endif

#
# Set up the Errata Report component.
#

ifneq ($(CPUS_ENABLE_ERRATA_REPORT),0)
        CPUS_DEFINES += HAVE_CPU_ERRATA_REPORT=1

        CPUS_SOURCES += lib/cpus/errata_report.c
endif

#
# Set up the workaround for CVE-2017-5715.
#

ifneq ($(CPUS_ENABLE_CVE_2017_5715_WORKAROUND),0)
        CPUS_DEFINES += HAVE_CPU_CVE_2017_5715_WORKAROUND=1

        CPUS_SOURCES += lib/cpus/$(ARCH)/wa_cve_2017_5715_bpiall.S
        CPUS_SOURCES += lib/cpus/$(ARCH)/wa_cve_2017_5715_mmu.S
endif

#
# Clean up after ourselves.
#

undefine CPUS_ENABLE_CPU_AMU
undefine CPUS_ENABLE_CPU_OPERATIONS
undefine CPUS_ENABLE_CVE_2017_5715_WORKAROUND
undefine CPUS_ENABLE_ERRATA_REPORT
