#
# Copyright (c) 2021-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk
include lib/libfdt/libfdt.mk

RESET_TO_BL31 := 1
ifeq (${RESET_TO_BL31}, 0)
$(error "This is a BL31-only port; RESET_TO_BL31 must be enabled")
endif

ifeq (${ENABLE_PIE}, 1)
override SEPARATE_CODE_AND_RODATA := 1
endif

CTX_INCLUDE_AARCH32_REGS := 0
ifeq (${CTX_INCLUDE_AARCH32_REGS}, 1)
$(error "This is an AArch64-only port; CTX_INCLUDE_AARCH32_REGS must be disabled")
endif

ifeq (${TRUSTED_BOARD_BOOT}, 1)
$(error "TRUSTED_BOARD_BOOT must be disabled")
endif

PRELOADED_BL33_BASE := 0x80080000

FPGA_PRELOADED_DTB_BASE := 0x80070000
$(eval $(call add_define,FPGA_PRELOADED_DTB_BASE))

FPGA_PRELOADED_CMD_LINE := 0x1000
$(eval $(call add_define,FPGA_PRELOADED_CMD_LINE))

ENABLE_AMU		:=	1

# Treating this as a memory-constrained port for now
USE_COHERENT_MEM	:=	0

# This can be overridden depending on CPU(s) used in the FPGA image
HW_ASSISTED_COHERENCY	:=	1

PL011_GENERIC_UART	:=	1

SUPPORT_UNKNOWN_MPID	?=	1

CPUS_SUPPORTED		+=	aem_generic

# select a different set of CPU files, depending on whether we compile for
# hardware assisted coherency cores or not
ifeq (${HW_ASSISTED_COHERENCY}, 0)
        # Cores used without DSU
        CPUS_SUPPORTED	+=	cortex_a35
        CPUS_SUPPORTED	+=	cortex_a53
        CPUS_SUPPORTED	+=	cortex_a57
        CPUS_SUPPORTED	+=	cortex_a72
        CPUS_SUPPORTED	+=	cortex_a73
else
        # AArch64-only cores
        CPUS_SUPPORTED	+=	cortex_a65
        CPUS_SUPPORTED	+=	cortex_a65ae
        CPUS_SUPPORTED	+=	cortex_a76
        CPUS_SUPPORTED	+=	cortex_a76ae
        CPUS_SUPPORTED	+=	cortex_a77
        CPUS_SUPPORTED	+=	cortex_a78
        CPUS_SUPPORTED	+=	cortex_a78_ae
        CPUS_SUPPORTED	+=	cortex_a78c
        CPUS_SUPPORTED	+=	cortex_a510
        CPUS_SUPPORTED	+=	cortex_a710
        CPUS_SUPPORTED	+=	cortex_a715
        CPUS_SUPPORTED	+=	cortex_x3
        CPUS_SUPPORTED	+=	neoverse_e1
        CPUS_SUPPORTED	+=	neoverse_n1
        CPUS_SUPPORTED	+=	neoverse_n2
        CPUS_SUPPORTED	+=	neoverse_v1

        # AArch64/AArch32 cores
        CPUS_SUPPORTED	+=	cortex_a55
        CPUS_SUPPORTED	+=	cortex_a75
endif

ifeq (${SUPPORT_UNKNOWN_MPID}, 1)
        # Add support for unknown/invalid MPIDs (aarch64 only)
        $(eval $(call add_define,SUPPORT_UNKNOWN_MPID))
        CPUS_SUPPORTED	+=	generic
endif

# Allow detection of GIC-600
GICV3_SUPPORT_GIC600	:=	1

GIC_ENABLE_V4_EXTN	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

FPGA_GIC_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/board/arm_fpga/fpga_gicv3.c

FDT_SOURCES		:=	fdts/arm_fpga.dts

PLAT_INCLUDES		:=	-Iplat/arm/board/arm_fpga/include

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/arm_fpga/${ARCH}/fpga_helpers.S

BL31_SOURCES		+=	common/fdt_fixup.c				\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/pl011/${ARCH}/pl011_console.S	\
				plat/common/plat_psci_common.c			\
				plat/arm/board/arm_fpga/fpga_pm.c			\
				plat/arm/board/arm_fpga/fpga_topology.c		\
				plat/arm/board/arm_fpga/fpga_console.c		\
				plat/arm/board/arm_fpga/fpga_bl31_setup.c		\
				${FPGA_GIC_SOURCES}

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

$(eval $(call MAKE_S,$(BUILD_PLAT),plat/arm/board/arm_fpga/rom_trampoline.S,bl31))
$(eval $(call MAKE_S,$(BUILD_PLAT),plat/arm/board/arm_fpga/kernel_trampoline.S,bl31))
$(eval $(call MAKE_LD,$(BUILD_PLAT)/build_axf.ld,plat/arm/board/arm_fpga/build_axf.ld.S,bl31))

bl31.axf: bl31 dtbs ${BUILD_PLAT}/rom_trampoline.o ${BUILD_PLAT}/kernel_trampoline.o ${BUILD_PLAT}/build_axf.ld
	$(ECHO) "  LD      $@"
	$(Q)$(LD) -T ${BUILD_PLAT}/build_axf.ld -L ${BUILD_PLAT} --strip-debug -s -n -o ${BUILD_PLAT}/bl31.axf

all: bl31.axf
