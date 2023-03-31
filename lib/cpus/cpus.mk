#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CPUS_SUPPORTED ?=

CPUS_CPU_HELPERS_ENABLED ?= 0
CPUS_DSU_HELPERS_ENABLED ?= 0
CPUS_CPUAMU_ENABLED ?= 0
CPUS_ERRATA_REPORT_ENABLED ?= 0
CPUS_CVE_2017_5715_WORKAROUND_ENABLED ?= 0

ifeq ($(CPUS_SUPPORTED),)
        $(error this platform has not configured any CPUs (`CPUS_SUPPORTED` is empty))
endif

ifneq ($(CPUS_DSU_HELPERS_ENABLED),0)
        ifneq ($(ARCH),aarch64)
                $(error DSU helpers are only supported in the AArch64 execution state (`CPUS_DSU_HELPERS_ENABLED=$(CPUS_DSU_HELPERS_ENABLED)` requires `ARCH=aarch64`, have `ARCH=$(ARCH)`))
        endif
endif

ifneq ($(CPUS_CPUAMU_ENABLED),0)
        ifeq ($(ENABLE_FEAT_AMU),0)
                $(error CPUAMU support requires that AMU support be enabled (`CPUS_CPUAMU_ENABLED=$(CPUS_CPUAMU_ENABLED)` requires `ENABLE_FEAT_AMU=1`, have `ENABLE_FEAT_AMU=$(ENABLE_FEAT_AMU)`))
        endif
endif

ifneq ($(CPUS_CVE_2017_5715_WORKAROUND_ENABLED),0)
        ifeq ($(WORKAROUND_CVE_2017_5715),0)
                $(error this image requires the CVE-2017-5715 CPU workaround, but the workaround is not enabled (`CPUS_CVE_2017_5715_WORKAROUND_ENABLED=$(CPUS_CVE_2017_5715_WORKAROUND_ENABLED)` requires `WORKAROUND_CVE_2017_5715=1`, have `WORKAROUND_CVE_2017_5715=$(WORKAROUND_CVE_2017_5715)`))
        endif

        ifneq ($(ARCH),aarch64)
                $(error the workaround for CVE-2017-5715 is only applicable in the AArch64 execution state (`CPUS_CVE_2017_5715_WORKAROUND_ENABLED=$(CPUS_CVE_2017_5715_WORKAROUND_ENABLED)` requires `ARCH=aarch64`, have `ARCH=$(ARCH)`))
        endif
endif

CPUS_WITH_HW_ASSISTED_COHERENCY := cortex_a55
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a65
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a65ae
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a75
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a76
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a76ae
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a77
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a78
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a78_ae
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a78c
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a510
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a710
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_a715
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_x1
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_x2
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_x3
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_hayes
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_hunter
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_hunter_elp_arm
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_chaberton
CPUS_WITH_HW_ASSISTED_COHERENCY += cortex_blackhawk
CPUS_WITH_HW_ASSISTED_COHERENCY += neoverse_e1
CPUS_WITH_HW_ASSISTED_COHERENCY += neoverse_n1
CPUS_WITH_HW_ASSISTED_COHERENCY += neoverse_n2
CPUS_WITH_HW_ASSISTED_COHERENCY += neoverse_v1
CPUS_WITH_HW_ASSISTED_COHERENCY += neoverse_v2
CPUS_WITH_HW_ASSISTED_COHERENCY += neoverse_poseidon
CPUS_WITH_HW_ASSISTED_COHERENCY += rainier

CPUS_SUPPORTED_WITH_HW_ASSISTED_COHERENCY := \
        $(filter $(CPUS_WITH_HW_ASSISTED_COHERENCY),$(CPUS_SUPPORTED))

ifneq ($(CPUS_SUPPORTED_WITH_HW_ASSISTED_COHERENCY),)
        ifeq ($(HW_ASSISTED_COHERENCY),0)
                $(error hardware coherency support must be enabled (via `HW_ASSISTED_COHERENCY`) to support these CPUs: $(CPUS_SUPPORTED_WITH_HW_ASSISTED_COHERENCY))
        endif
endif

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

CPUS_SUPPORTED_WITHOUT_AARCH32_EXECUTION_STATE := \
        $(filter-out $(CPUS_WITH_AARCH32_EXECUTION_STATE),$(CPUS_SUPPORTED))

ifneq ($(CTX_INCLUDE_AARCH32_REGS),0)
        ifneq ($(CPUS_SUPPORTED_WITHOUT_AARCH32_EXECUTION_STATE),)
                $(error there are CPUs enabled (via `CPUS_SUPPORTED`) which do not support the AArch32 execution state (required by `CTX_INCLUDE_AARCH32_REGS=$(CTX_INCLUDE_AARCH32_REGS)`): $(CPUS_SUPPORTED_WITHOUT_AARCH32_EXECUTION_STATE))
        endif
endif

CPUS_DEFINES += $(addprefix HAVE_CPU_,$(call uppercase,$(CPUS_SUPPORTED)))

ifneq ($(filter neoverse_%,$(CPUS_SUPPORTED)),)
        CPUS_DEFINES += HAVE_CPU_NEOVERSE_Nx
endif

CPUS_INCLUDE_DIRS := include/lib/cpus
CPUS_INCLUDE_DIRS += include/lib/cpus/$(ARCH)

CPUS_SOURCES := $(addprefix lib/cpus/$(ARCH)/,$(CPUS_SUPPORTED))
CPUS_SOURCES := $(addsuffix .S,$(CPUS_SOURCES))

ifneq ($(filter neoverse_%,$(CPUS_SUPPORTED)),)
        CPUS_SOURCES += lib/cpus/$(ARCH)/neoverse_n_common.S
endif

ifneq ($(CPUS_CPU_HELPERS_ENABLED),0)
        CPUS_SOURCES += lib/cpus/$(ARCH)/cpu_helpers.S
endif

ifneq ($(CPUS_DSU_HELPERS_ENABLED),0)
        CPUS_SOURCES += lib/cpus/$(ARCH)/dsu_helpers.S
endif

CPUS_WITH_CPUAMU := cortex_a75
CPUS_WITH_CPUAMU += neoverse_n1

ifneq ($(CPUS_CPUAMU_ENABLED),0)
        CPUS_SUPPORTED_WITH_CPUAMU := \
                $(filter $(CPUS_WITH_CPUAMU),$(CPUS_SUPPORTED))

        ifneq ($(CPUS_SUPPORTED_WITH_CPUAMU),)
                CPUS_SOURCES += lib/cpus/$(ARCH)/cpuamu.c
                CPUS_SOURCES += lib/cpus/$(ARCH)/cpuamu_helpers.S

                ifneq ($(HW_ASSISTED_COHERENCY),0)
                        CPUS_SOURCES += $(patsubst %,lib/cpus/$(ARCH)/%_pubsub.c,$(CPUS_SUPPORTED_WITH_CPUAMU))
                endif
        endif
endif

ifneq ($(CPUS_ERRATA_REPORT_ENABLED),0)
        CPUS_SOURCES += lib/cpus/errata_report.c
endif

ifneq ($(CPUS_CVE_2017_5715_WORKAROUND_ENABLED),0)
        CPUS_SOURCES += lib/cpus/$(ARCH)/wa_cve_2017_5715_bpiall.S
        CPUS_SOURCES += lib/cpus/$(ARCH)/wa_cve_2017_5715_mmu.S
endif

undefine CPUS_CPU_HELPERS_ENABLED
undefine CPUS_DSU_HELPERS_ENABLED
undefine CPUS_CPUAMU_ENABLED
undefine CPUS_ERRATA_REPORT_ENABLED
undefine CPUS_CVE_2017_5715_WORKAROUND_ENABLED
