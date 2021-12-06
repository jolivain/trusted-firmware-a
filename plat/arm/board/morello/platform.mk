#
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MORELLO_BASE		:=	plat/arm/board/morello

INTERCONNECT_SOURCES	:=	${MORELLO_BASE}/morello_interconnect.c

PLAT_INCLUDES		:=	-I${MORELLO_BASE}/include

MORELLO_CPU_SOURCES	:=	lib/cpus/aarch64/rainier.S

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

MORELLO_GIC_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c		\

PLAT_BL_COMMON_SOURCES	:=	${MORELLO_BASE}/morello_plat.c		\
				${MORELLO_BASE}/aarch64/morello_helper.S

BL1_SOURCES		:=	${MORELLO_CPU_SOURCES}			\
				${INTERCONNECT_SOURCES}			\
				${MORELLO_BASE}/morello_err.c		\
				${MORELLO_BASE}/morello_trusted_boot.c	\
				${MORELLO_BASE}/morello_bl1_setup.c	\
				drivers/arm/sbsa/sbsa.c

BL2_SOURCES		:=	${MORELLO_BASE}/morello_security.c	\
				${MORELLO_BASE}/morello_err.c		\
				${MORELLO_BASE}/morello_trusted_boot.c	\
				lib/utils/mem_region.c			\
				${MORELLO_BASE}/morello_bl2_setup.c

BL31_SOURCES		:=	${MORELLO_CPU_SOURCES}			\
				${INTERCONNECT_SOURCES}			\
				${MORELLO_GIC_SOURCES}			\
				${MORELLO_BASE}/morello_bl31_setup.c	\
				${MORELLO_BASE}/morello_topology.c	\
				${MORELLO_BASE}/morello_security.c	\
				drivers/arm/css/sds/sds.c

FDT_SOURCES		+=	fdts/morello-fvp.dts				\
				fdts/morello-soc.dts				\
				${MORELLO_BASE}/fdts/morello_fw_config.dts	\
				${MORELLO_BASE}/fdts/morello_tb_fw_config.dts	\

FW_CONFIG		:=	${BUILD_PLAT}/fdts/morello_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/morello_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

# TF-A not required to load the SCP Images
override CSS_LOAD_SCP_IMAGES		:=	0

override NEED_BL2U			:=	no

# 32 bit mode not supported
override CTX_INCLUDE_AARCH32_REGS	:=	0

override ARM_PLAT_MT			:=	1

# Select SCMI/SDS drivers instead of SCPI/BOM driver for communicating with the
# SCP during power management operations and for SCP RAM Firmware transfer.
CSS_USE_SCMI_SDS_DRIVER			:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY			:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM			:=	0

# Add PLAT_FVP flag to differentiate between Morello FVP and Morello SoC platform
$(eval $(call assert_boolean,PLAT_FVP))
$(eval $(call add_define,PLAT_FVP))

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk
