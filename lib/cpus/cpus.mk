#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cpus-needing-hw-assisted-coherency := cortex_a55
cpus-needing-hw-assisted-coherency += cortex_a65
cpus-needing-hw-assisted-coherency += cortex_a65ae
cpus-needing-hw-assisted-coherency += cortex_a75
cpus-needing-hw-assisted-coherency += cortex_a76
cpus-needing-hw-assisted-coherency += cortex_a76ae
cpus-needing-hw-assisted-coherency += cortex_a77
cpus-needing-hw-assisted-coherency += cortex_a78
cpus-needing-hw-assisted-coherency += cortex_a78_ae
cpus-needing-hw-assisted-coherency += cortex_a78c
cpus-needing-hw-assisted-coherency += cortex_a510
cpus-needing-hw-assisted-coherency += cortex_a710
cpus-needing-hw-assisted-coherency += cortex_a715
cpus-needing-hw-assisted-coherency += cortex_a720
cpus-needing-hw-assisted-coherency += cortex_x1
cpus-needing-hw-assisted-coherency += cortex_x2
cpus-needing-hw-assisted-coherency += cortex_x3
cpus-needing-hw-assisted-coherency += cortex_x4
cpus-needing-hw-assisted-coherency += cortex_blackhawk
cpus-needing-hw-assisted-coherency += cortex_chaberton
cpus-needing-hw-assisted-coherency += cortex_gelas
cpus-needing-hw-assisted-coherency += cortex_hayes
cpus-needing-hw-assisted-coherency += cortex_hunter
cpus-needing-hw-assisted-coherency += cortex_hunter_elp_arm
cpus-needing-hw-assisted-coherency += neoverse_n1
cpus-needing-hw-assisted-coherency += neoverse_n2
cpus-needing-hw-assisted-coherency += neoverse_e1
cpus-needing-hw-assisted-coherency += neoverse_v1
cpus-needing-hw-assisted-coherency += neoverse_v2
cpus-needing-hw-assisted-coherency += neoverse_hermes
cpus-needing-hw-assisted-coherency += neoverse_poseidon
cpus-needing-hw-assisted-coherency += nevis
cpus-needing-hw-assisted-coherency += rainier

cpus-with-aarch32 := aem_generic
cpus-with-aarch32 += cortex_a35
cpus-with-aarch32 += cortex_a53
cpus-with-aarch32 += cortex_a55
cpus-with-aarch32 += cortex_a57
cpus-with-aarch32 += cortex_a72
cpus-with-aarch32 += cortex_a73
cpus-with-aarch32 += cortex_a75
cpus-with-aarch32 += cortex_a78
cpus-with-aarch32 += cortex_a78_ae
cpus-with-aarch32 += cortex_a78c
cpus-with-aarch32 += denver
cpus-with-aarch32 += generic

cpus-with-impdef-amu := cortex_a75
cpus-with-impdef-amu += neoverse_n1

cpus-needing-dsu-helpers := cortex_a55
cpus-needing-dsu-helpers += cortex_a65
cpus-needing-dsu-helpers += cortex_a65ae
cpus-needing-dsu-helpers += cortex_a75
cpus-needing-dsu-helpers += cortex_a76
cpus-needing-dsu-helpers += cortex_a510
cpus-needing-dsu-helpers += cortex_a710
cpus-needing-dsu-helpers += cortex_x2
cpus-needing-dsu-helpers += neoverse_n1
cpus-needing-dsu-helpers += neoverse_n2
cpus-needing-dsu-helpers += neoverse_e1

define cpus-config
        $(1)-cpus-enable-cpu-operations ?= 0
        $(1)-cpus-enable-cve-2017-5715-workaround ?= 0
        $(1)-cpus-enable-errata-report ?= 0
        $(1)-cpus-enable-impdef-amu ?= 0

        $(1)-cpus-derived-sources := $$(sort $$(filter lib/cpus/%, \
                $$($(1)-cpus-bl-sources) \
        ))

        $(1)-cpus-derived := $$(sort $$(filter-out %_helpers cpuamu wa_%, \
                $$(basename $$(notdir \
                        $$(filter lib/cpus/aarch32/% lib/cpus/aarch64/%, \
                                $$($(1)-cpus-derived-sources) \
                        ) \
                )) \
        ))

        ifneq ($$($(1)-cpus-derived-sources),)
                $$(warning $(1): this image has explicitly specified CPU support library source files: $$($(1)-cpus-derived-sources))
                $$(warning $(1): CPUs should instead be specified directly via `CPUS` (e.g. `CPUS := $$($(1)-cpus-derived)`).)
        endif

        $(1)-cpus += $$($(1)-cpus-derived)

        ifneq ($$($(1)-cpus-enable-impdef-amu),0)
                ifeq ($$(ENABLE_FEAT_AMU),0)
                        $$(error the IMPDEF AMU component requires that architectural AMU feature support be enabled (`$(1)-cpus-enable-impdef-amu=$$($(1)-cpus-enable-impdef-amu)` requires `ENABLE_FEAT_AMU=1`, have `ENABLE_FEAT_AMU=0`))
                endif
        endif

        ifneq ($$($(1)-cpus-enable-cve-2017-5715-workaround),0)
                ifeq ($$(WORKAROUND_CVE_2017_5715),0)
                        $$(error the CVE-2017-5715 workaround requires that it is enabled globally (`$(1)-cpus-enable-cve-2017-5715-workaround=$$($(1)-cpus-enable-cve-2017-5715-workaround)` requires `WORKAROUND_CVE_2017_5715=1`, have `WORKAROUND_CVE_2017_5715=0`))
                endif

                ifneq ($$(ARCH),aarch64)
                        $$(error the CVE-2017-5715 workaround is only applicable when the AArch64 execution state is available (`$(1)-cpus-enable-cve-2017-5715-workaround=$$($(1)-cpus-enable-cve-2017-5715-workaround)` requires `ARCH=aarch64`, have `ARCH=$$(ARCH)`))
                endif
        endif

        $(1)-cpus := $$(sort $$($(1)-cpus))
        $(1)-cpus-needing-hw-assisted-coherency := \
                $$(filter $$(cpus-needing-hw-assisted-coherency),$$($(1)-cpus))

        ifneq ($$($(1)-cpus-needing-hw-assisted-coherency),)
                ifeq ($$(HW_ASSISTED_COHERENCY),0)
                        $$(error hardware-assisted coherency support must be enabled (have `HW_ASSISTED_COHERENCY=0`, need `HW_ASSISTED_COHERENCY=1`) to support these CPUs: $$($(1)-cpus-needing-hw-assisted-coherency))
                endif
        endif

        $(1)-cpus-enabled-without-aarch32 := \
                $$(filter-out $$(cpus-with-aarch32),$$($(1)-cpus))

        ifeq ($$(ARCH),aarch64)
                ifneq ($$(CTX_INCLUDE_AARCH32_REGS),0)
                        ifneq ($$($(1)-cpus-enabled-without-aarch32),)
                                $$(error there are AArch64 CPUs enabled (via `$(1)-cpus`) which do not support the AArch32 execution state (required by `CTX_INCLUDE_AARCH32_REGS=$$(CTX_INCLUDE_AARCH32_REGS)`): $$($(1)-cpus-enabled-without-aarch32))
                        endif
                endif
        endif

        $(1)-cpus-need-impdef-amu := 0
        $(1)-cpus-enabled-with-impdef-amu := \
                $$(filter $$(cpus-with-impdef-amu),$$($(1)-cpus))

        ifneq ($$($(1)-cpus-enable-impdef-amu),0)
                ifneq ($$($(1)-cpus-enabled-with-impdef-amu),)
                        $(1)-cpus-need-impdef-amu := 1
                endif
        endif

        $(1)-cpus-need-dsu-helpers := 0
        $(1)-cpus-enabled-with-dsu-helpers := \
                $$(filter $$(cpus-needing-dsu-helpers),$$($(1)-cpus))

        ifneq ($$($(1)-cpus-enabled-with-dsu-helpers),)
                $(1)-cpus-need-dsu-helpers := 1
        endif

        $(1)-cpus-defines += $$(patsubst %,HAVE_CPU_%=1,$$(call uppercase,$$($(1)-cpus)))

        $(1)-cpus-include-dirs += include/lib/cpus
        $(1)-cpus-include-dirs += include/lib/cpus/$$(ARCH)

        ifneq ($$(filter neoverse_n%,$$($(1)-cpus)),)
                $(1)-cpus-defines += HAVE_CPU_NEOVERSE_Nx=1
        endif

        ifneq ($$($(1)-cpus-enable-cpu-operations),0)
                $(1)-cpus-defines += HAVE_CPU_OPERATIONS=1

                $(1)-cpus-sources += lib/cpus/$(ARCH)/cpu_helpers.S
                $(1)-cpus-sources += $$(patsubst %,lib/cpus/$$(ARCH)/%.S,$$($(1)-cpus))

                ifneq ($$($(1)-cpus-need-dsu-helpers),0)
                        $(1)-cpus-sources += lib/cpus/$$(ARCH)/dsu_helpers.S
                endif

                ifneq ($$(filter neoverse_n%,$$($(1)-cpus)),)
                        $(1)-cpus-sources += lib/cpus/$$(ARCH)/neoverse_n_common.S
                endif
        endif

        ifneq ($$($(1)-cpus-enable-cve-2017-5715-workaround),0)
                $(1)-cpus-defines += HAVE_CPU_CVE_2017_5715_WORKAROUND=1

                $(1)-cpus-sources += lib/cpus/$$(ARCH)/wa_cve_2017_5715_bpiall.S
                $(1)-cpus-sources += lib/cpus/$$(ARCH)/wa_cve_2017_5715_mmu.S
        endif

        ifneq ($$($(1)-cpus-enable-errata-report),0)
                $(1)-cpus-defines += HAVE_CPU_ERRATA_REPORT=1
                $(1)-cpus-sources += lib/cpus/errata_report.c
        endif

        ifneq ($$($(1)-cpus-need-impdef-amu),0)
                $(1)-cpus-defines += HAVE_CPU_IMPDEF_AMU=1

                $(1)-cpus-sources += lib/cpus/$$(ARCH)/cpuamu.c
                $(1)-cpus-sources += lib/cpus/$$(ARCH)/cpuamu_helpers.S
                $(1)-cpus-sources += $$(patsubst %,lib/cpus/$$(ARCH)/%_pubsub.c,$$($(1)-cpus-enabled-with-impdef-amu))
        endif
endef
