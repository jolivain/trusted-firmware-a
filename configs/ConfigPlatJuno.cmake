#-------------------------------------------------------------------------------
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIR not defined")
endif()

# Include framework files
include(Common/Group)

group_new(NAME hw_plat)

#Config from plat/arm/board/juno/platform.mk
#-------------------------------------------------------------------------------
group_add(NAME hw_plat TYPE INCLUDE KEY ${PROJECT_SOURCE_DIR}/plat/arm/board/juno/include)

group_add(NAME hw_plat TYPE CONFIG KEY DTS_TB_FW_CONFIG VAL ${PROJECT_SOURCE_DIR}/plat/arm/board/juno/fdts/juno_tb_fw_config.dts)

# Select SCMI/SDS drivers instead of SCPI/BOM driver for communicating with the
# SCP during power management operations and for SCP RAM Firmware transfer.
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY CSS_USE_SCMI_SDS_DRIVER VAL 1)

# Flag to enable support for AArch32 state on JUNO
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY JUNO_AARCH32_EL3_RUNTIME VAL 0)

# Flag to enable support for TZMP1 on JUNO
#group_add(NAME hw_plat TYPE CONFIG DEFINE KEY JUNO_TZMP1 VAL 0) TODO: why is this checked with #ifdef instead of #if?

# Enable Activity Monitor Unit extensions by default
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY ENABLE_AMU VAL 1)

# Enable dynamic mitigation support by default
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY DYNAMIC_WORKAROUND_CVE_2018_3639 VAL 1)

group_add(NAME hw_plat TYPE CONFIG DEFINE KEY RESET_TO_BL31 VAL 0)

group_add(NAME hw_plat TYPE DEFINE KEY COLD_BOOT_SINGLE_CPU VAL 0)
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY BL2_AT_EL3 VAL 0)

#Config from plat/arm/board/common/board_common.mk
#-------------------------------------------------------------------------------

#Config from plat/arm/common/arm_common.mk
#-------------------------------------------------------------------------------
group_add(NAME hw_plat TYPE INCLUDE KEY ${PROJECT_SOURCE_DIR}/include/plat/arm/common/aarch64)

# On ARM standard platorms, the TSP can execute from Trusted SRAM, Trusted
# DRAM (if available) or the TZC secured area of DRAM.
# TZC secured DRAM is the default.
group_add(NAME hw_plat TYPE CONFIG KEY ARM_TSP_RAM_LOCATION VAL dram) #TODO: unused?
group_add(NAME hw_plat TYPE DEFINE KEY ARM_TSP_RAM_LOCATION_ID VAL ARM_DRAM_ID)

group_add(NAME hw_plat TYPE DEFINE KEY ARM_BL31_IN_DRAM VAL 0)

# For the original power-state parameter format, the State-ID can be encoded
# according to the recommended encoding or zero. This flag determines which
# State-ID encoding to be parsed.
group_add(NAME hw_plat TYPE DEFINE KEY ARM_RECOM_STATE_ID_ENC VAL 0)

# By default, Trusted Watchdog is always enabled unless SPIN_ON_BL1_EXIT is set
group_add(NAME hw_plat TYPE DEFINE KEY ARM_DISABLE_TRUSTED_WDOG VAL 0)

group_add(NAME hw_plat TYPE DEFINE KEY ARM_CONFIG_CNTACR VAL 1)

group_add(NAME hw_plat TYPE DEFINE KEY ARM_PLAT_MT VAL 1)

# Use translation tables library v2 by default
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY ARM_XLAT_TABLES_LIB_V1 VAL 0)

# Don't have the Linux kernel as a BL33 image by default
group_add(NAME hw_plat TYPE DEFINE KEY ARM_LINUX_KERNEL_AS_BL33 VAL 0)

# Use an implementation of SHA-256 with a smaller memory footprint but reduced
# speed.
group_add(NAME hw_plat TYPE DEFINE KEY MBEDTLS_SHA256_SMALLER)

#TODO: BL32_EXTRA1, BL32_EXTRA2

# Enable PSCI_STAT_COUNT/RESIDENCY APIs on ARM platforms
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY ENABLE_PSCI_STAT VAL 1)
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY ENABLE_PMF VAL 1)

# Whether code and read-only data should be put on separate memory pages.
# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
group_add(NAME hw_plat TYPE DEFINE KEY SEPARATE_CODE_AND_RODATA VAL 1)

# Disable ARM Cryptocell by default
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY ARM_CRYPTOCELL_INTEG VAL 0)

#Config from plat/arm/soc/common/soc_css.cmake
#-------------------------------------------------------------------------------

#Config from plat/arm/css/common/css_common.cmake
#-------------------------------------------------------------------------------
group_add(NAME hw_plat TYPE INCLUDE KEY ${PROJECT_SOURCE_DIR}/include/plat/arm/css/common/aarch64)

# By default, SCP images are needed by CSS platforms.
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY CSS_LOAD_SCP_IMAGES VAL 1)
group_add(NAME hw_plat TYPE CONFIG KEY NEED_SCP_BL2 VAL yes) #TODO: fiptool args

# Enable option to detect whether the SCP ROM firmware in use predates version
# 1.7.0 and therefore, is incompatible.
group_add(NAME hw_plat TYPE DEFINE KEY CSS_DETECT_PRE_1_7_0_SCP VAL 1)

# Process CSS_NON_SECURE_UART flag
# This undocumented build option is only to enable debug access to the UART
# from non secure code, which is useful on some platforms.
# Default (obviously) is off.
group_add(NAME hw_plat TYPE DEFINE KEY CSS_NON_SECURE_UART VAL 0)

#Config from make_helpers/defaults.mk: moving platform specific configs here
#-------------------------------------------------------------------------------
# Build flag to include AArch32 registers in cpu context save and restore during
# world switch. This flag must be set to 0 for AArch64-only platforms.
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY CTX_INCLUDE_AARCH32_REGS VAL 1)

# Whether system coherency is managed in hardware, without explicit software
# operations.
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY HW_ASSISTED_COHERENCY VAL 0)

#Config from lib/cpus/cpu-ops.mk TODO: move erratas somewhere else?
#-------------------------------------------------------------------------------
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY WORKAROUND_CVE_2017_5715 VAL 1)
group_add(NAME hw_plat TYPE CONFIG DEFINE KEY WORKAROUND_CVE_2018_3639 VAL 1)
#group_add(NAME hw_plat TYPE CONFIG DEFINE KEY DYNAMIC_WORKAROUND_CVE_2018_3639 VAL 0) TODO: enabled in fvp

# Cortex A57 specific optimisation to skip L1 cache flush when
# cluster is powered down.
group_add(NAME hw_plat TYPE DEFINE KEY SKIP_A57_L1_FLUSH_PWR_DWN VAL 1)

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
group_add(NAME hw_plat TYPE DEFINE KEY A53_DISABLE_NON_TEMPORAL_HINT VAL 1)

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
group_add(NAME hw_plat TYPE DEFINE KEY A57_DISABLE_NON_TEMPORAL_HINT VAL 1)

# CPU Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 794073 workaround when disabling mmu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A9_794073 VAL 0)

# Flag to apply erratum 816470 workaround during power down. This erratum
# applies only to revision >= r3p0 of the Cortex A15 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A15_816470 VAL 0)

# Flag to apply erratum 827671 workaround during reset. This erratum applies
# only to revision >= r3p0 of the Cortex A15 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A15_827671 VAL 0)

# Flag to apply erratum 852421 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A17 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A17_852421 VAL 0)

# Flag to apply erratum 852423 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A17 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A17_852423 VAL 0)

# Flag to apply erratum 855472 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A35 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A35_855472 VAL 0)

# Flag to apply erratum 819472 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A53 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_819472 VAL 1)

# Flag to apply erratum 824069 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_824069 VAL 1)

# Flag to apply erratum 826319 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_826319 VAL 1)

# Flag to apply erratum 827319 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_827319 VAL 1)

# Flag to apply erratum 835769 workaround at compile and link time.  This
# erratum applies to revision <= r0p4 of the Cortex A53 cpu. Enabling this
# workaround can lead the linker to create "*.stub" sections.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_835769 VAL 1)

# Flag to apply erratum 836870 workaround during reset. This erratum applies
# only to revision <= r0p3 of the Cortex A53 cpu. From r0p4 and onwards, this
# erratum workaround is enabled by default in hardware.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_836870 VAL 1)

# Flag to apply erratum 843419 workaround at link time.
# This erratum applies to revision <= r0p4 of the Cortex A53 cpu. Enabling this
# workaround could lead the linker to emit "*.stub" sections which are 4kB
# aligned.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_843419 VAL 1)

# Flag to apply errata 855873 during reset. This errata applies to all
# revisions of the Cortex A53 CPU, but this firmware workaround only works
# for revisions r0p3 and higher. Earlier revisions are taken care
# of by the rich OS.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A53_855873 VAL 1)

# Flag to apply erratum 768277 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A55_768277 VAL 0)

# Flag to apply erratum 778703 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A55_778703 VAL 0)

# Flag to apply erratum 798797 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A55 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A55_798797 VAL 0)

# Flag to apply erratum 846532 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A55 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A55_846532 VAL 0)

# Flag to apply erratum 903758 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A55 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A55_903758 VAL 0)

# Flag to apply erratum 1221012 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A55 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A55_1221012 VAL 0)

# Flag to apply erratum 806969 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_806969 VAL 0)

# Flag to apply erratum 813419 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_813419 VAL 1)

# Flag to apply erratum 813420  workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_813420 VAL 1)

# Flag to apply erratum 814670  workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_814670 VAL 1)

# Flag to apply erratum 817169 workaround during power down. This erratum
# applies only to revision <= r0p1 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_817169 VAL 1)

# Flag to apply erratum 826974 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_826974 VAL 1)

# Flag to apply erratum 826977 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_826977 VAL 1)

# Flag to apply erratum 828024 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_828024 VAL 1)

# Flag to apply erratum 829520 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_829520 VAL 1)

# Flag to apply erratum 833471 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_833471 VAL 1)

# Flag to apply erratum 855972 workaround during reset. This erratum applies
# only to revision <= r1p3 of the Cortex A57 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A57_859972 VAL 0)

# Flag to apply erratum 855971 workaround during reset. This erratum applies
# only to revision <= r0p3 of the Cortex A72 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A72_859971 VAL 0)

# Flag to apply erratum 852427 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A73 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A73_852427 VAL 0)

# Flag to apply erratum 855423 workaround during reset. This erratum applies
# only to revision <= r0p1 of the Cortex A73 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A73_855423 VAL 0)

# Flag to apply erratum 764081 workaround during reset. This erratum applies
# only to revision <= r0p0 of the Cortex A75 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A75_764081 VAL 0)

# Flag to apply erratum 790748 workaround during reset. This erratum applies
# only to revision <= r0p0 of the Cortex A75 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A75_790748 VAL 0)

# Flag to apply erratum 1073348 workaround during reset. This erratum applies
# only to revision <= r1p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1073348 VAL 0)

# Flag to apply erratum 1130799 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1130799 VAL 0)

# Flag to apply erratum 1220197 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1220197 VAL 0)

# Flag to apply erratum 1257314 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1257314 VAL 0)

# Flag to apply erratum 1262606 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1262606 VAL 0)

# Flag to apply erratum 1262888 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1262888 VAL 0)

# Flag to apply erratum 1275112 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1275112 VAL 0)

# Flag to apply erratum 1286807 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Cortex A76 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_A76_1286807 VAL 0)

# Flag to apply T32 CLREX workaround during reset. This erratum applies
# only to r0p0 and r1p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1043202 VAL 1)

# Flag to apply erratum 1073348 workaround during reset. This erratum applies
# only to revision r0p0 and r1p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1073348 VAL 0)

# Flag to apply erratum 1130799 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1130799 VAL 0)

# Flag to apply erratum 1165347 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1165347 VAL 0)

# Flag to apply erratum 1207823 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1207823 VAL 0)

# Flag to apply erratum 1220197 workaround during reset. This erratum applies
# only to revision <= r2p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1220197 VAL 0)

# Flag to apply erratum 1257314 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1257314 VAL 0)

# Flag to apply erratum 1262606 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1262606 VAL 0)

# Flag to apply erratum 1262888 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1262888 VAL 0)

# Flag to apply erratum 1275112 workaround during reset. This erratum applies
# only to revision <= r3p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1275112 VAL 0)

# Flag to apply erratum 1315703 workaround during reset. This erratum applies
# to revisions before r3p1 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1315703 VAL 1)

# Flag to apply erratum 1542419 workaround during reset. This erratum applies
# to revisions r3p0 - r4p0 of the Neoverse N1 cpu.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_N1_1542419 VAL 0)

# Flag to apply DSU erratum 798953. This erratum applies to DSUs revision r0p0.
# Applying the workaround results in higher DSU power consumption on idle.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_DSU_798953 VAL 0)

# Flag to apply DSU erratum 936184. This erratum applies to DSUs containing
# the ACP interface and revision < r2p0. Applying the workaround results in
# higher DSU power consumption on idle.
group_add(NAME hw_plat TYPE DEFINE KEY ERRATA_DSU_936184 VAL 0)

group_add(NAME hw_plat TYPE CFLAG KEY -mfix-cortex-a53-843419)
group_add(NAME hw_plat TYPE LDFLAG KEY -Wl,--fix-cortex-a53-843419)

group_add(NAME hw_plat TYPE CFLAG KEY -mfix-cortex-a53-835769)
group_add(NAME hw_plat TYPE LDFLAG KEY -Wl,--fix-cortex-a53-835769)
