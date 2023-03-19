#
# Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
# Copyright (c) 2017 Nuvoton Technology Corp.
# SPDX-License-Identifier: BSD-3-Clause
#

#This is a debug flag for bring-up. It allows reducing CPU numbers 
#SECONDARY_BRINGUP	:=	1
RESET_TO_BL31		:=	1
PMD_SPM_AT_SEL2		:= 0
#temporaru until the RAM size is reduced 
USE_COHERENT_MEM	:=	1


$(eval $(call add_define,RESET_TO_BL31))

ifeq (${ARCH}, aarch64)
  # On ARM standard platorms, the TSP can execute from Trusted SRAM, Trusted
  # DRAM (if available) or the TZC secured area of DRAM.
  # TZC secured DRAM is the default.

  ARM_TSP_RAM_LOCATION	?=	dram

  ifeq (${ARM_TSP_RAM_LOCATION}, tsram)
    ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_SRAM_ID
  else ifeq (${ARM_TSP_RAM_LOCATION}, tdram)
    ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_DRAM_ID
  else ifeq (${ARM_TSP_RAM_LOCATION}, dram)
    ARM_TSP_RAM_LOCATION_ID = ARM_DRAM_ID
  else
    $(error "Unsupported ARM_TSP_RAM_LOCATION value")
  endif

  # Process flags
  # Process ARM_BL31_IN_DRAM flag
  #ARM_BL31_IN_DRAM		:=	1
  $(eval $(call assert_boolean,ARM_BL31_IN_DRAM))
  # Process ARM_BL31_IN_SRAM flag
  ARM_BL31_IN_DRAM		:=	0
  
  $(eval $(call add_define,ARM_BL31_IN_DRAM))
else
  ARM_TSP_RAM_LOCATION_ID = ARM_TRUSTED_SRAM_ID
endif


$(eval $(call add_define,ARM_TSP_RAM_LOCATION_ID))


# For the original power-state parameter format, the State-ID can be encoded
# according to the recommended encoding or zero. This flag determines which
# State-ID encoding to be parsed.
ARM_RECOM_STATE_ID_ENC := 0

# If the PSCI_EXTENDED_STATE_ID is set, then ARM_RECOM_STATE_ID_ENC need to
# be set. Else throw a build error.
ifeq (${PSCI_EXTENDED_STATE_ID}, 1)
  ifeq (${ARM_RECOM_STATE_ID_ENC}, 0)
    $(error Build option ARM_RECOM_STATE_ID_ENC needs to be set if \
            PSCI_EXTENDED_STATE_ID is set for ARM platforms)
  endif
endif


# Process ARM_RECOM_STATE_ID_ENC flag
$(eval $(call assert_boolean,ARM_RECOM_STATE_ID_ENC))
$(eval $(call add_define,ARM_RECOM_STATE_ID_ENC))

# Process ARM_DISABLE_TRUSTED_WDOG flag
# By default, Trusted Watchdog is always enabled unless SPIN_ON_BL1_EXIT is set
ARM_DISABLE_TRUSTED_WDOG	:=	0
ifeq (${SPIN_ON_BL1_EXIT}, 1)
ARM_DISABLE_TRUSTED_WDOG	:=	1
endif
$(eval $(call assert_boolean,ARM_DISABLE_TRUSTED_WDOG))
$(eval $(call add_define,ARM_DISABLE_TRUSTED_WDOG))

# Process ARM_CONFIG_CNTACR
ARM_CONFIG_CNTACR		:=	1
$(eval $(call assert_boolean,ARM_CONFIG_CNTACR))
$(eval $(call add_define,ARM_CONFIG_CNTACR))

# Process ARM_BL31_IN_DRAM flag
ARM_BL31_IN_DRAM		:=	0
$(eval $(call assert_boolean,ARM_BL31_IN_DRAM))
$(eval $(call add_define,ARM_BL31_IN_DRAM))

# Process ARM_PLAT_MT flag
ARM_PLAT_MT			:=	0
$(eval $(call assert_boolean,ARM_PLAT_MT))
$(eval $(call add_define,ARM_PLAT_MT))

# Use translation tables library v2 by default
ARM_XLAT_TABLES_LIB_V1		:=	0
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))

# Don't have the Linux kernel as a BL33 image by default
ARM_LINUX_KERNEL_AS_BL33	:=	0
$(eval $(call assert_boolean,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))

ifeq (${ARM_LINUX_KERNEL_AS_BL33},1)
  ifeq (${ARCH},aarch64)
    ifneq (${RESET_TO_BL31},1)
      $(error "ARM_LINUX_KERNEL_AS_BL33 is only available if RESET_TO_BL31=1.")
    endif
  else
    ifneq (${RESET_TO_SP_MIN},1)
      $(error "ARM_LINUX_KERNEL_AS_BL33 is only available if RESET_TO_SP_MIN=1.")
    endif
  endif
  ifndef PRELOADED_BL33_BASE
    $(error "PRELOADED_BL33_BASE must be set if ARM_LINUX_KERNEL_AS_BL33 is used.")
  endif
  ifndef ARM_PRELOADED_DTB_BASE
    $(error "ARM_PRELOADED_DTB_BASE must be set if ARM_LINUX_KERNEL_AS_BL33 is used.")
  endif
  $(eval $(call add_define,ARM_PRELOADED_DTB_BASE))
endif


# In order to support SEPARATE_NOBITS_REGION for Arm platforms, we need to load
# BL31 PROGBITS into secure DRAM space and BL31 NOBITS into SRAM. Hence mandate
# the build to require that ARM_BL31_IN_DRAM is enabled as well.
ifeq ($(SEPARATE_NOBITS_REGION),1)
    ifneq ($(ARM_BL31_IN_DRAM),1)
         $(error For SEPARATE_NOBITS_REGION, ARM_BL31_IN_DRAM must be enabled)
    endif
    ifneq ($(RECLAIM_INIT_CODE),0)
          $(error For SEPARATE_NOBITS_REGION, RECLAIM_INIT_CODE cannot be supported)
    endif
endif


# Disable ARM Cryptocell by default
ARM_CRYPTOCELL_INTEG		:=	0
$(eval $(call assert_boolean,ARM_CRYPTOCELL_INTEG))
$(eval $(call add_define,ARM_CRYPTOCELL_INTEG))


# Enable PIE support for RESET_TO_BL31 case
ifeq (${RESET_TO_BL31},1)
    ENABLE_PIE			:=	1
endif
 
 
# CryptoCell integration relies on coherent buffers for passing data from
# the AP CPU to the CryptoCell

ifeq (${ARM_CRYPTOCELL_INTEG},1)
    ifeq (${USE_COHERENT_MEM},0)
        $(error "ARM_CRYPTOCELL_INTEG needs USE_COHERENT_MEM to be set.")
    endif
endif



ifeq (${ARCH}, aarch64)
PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/aarch64
endif

PLAT_INCLUDES		:=	-Iinclude/plat/nuvoton/common		\
				-Iinclude/plat/nuvoton/npcm845x		\
				-Iinclude/drivers/nuvoton/common		\
				-Iinclude/drivers/nuvoton/npcm845x		\

# Include GICv3 driver files
include drivers/arm/gic/v2/gicv2.mk

NPCM850_GIC_SOURCES	:=			${GICV2_SOURCES}			
				

BL31_SOURCES	+=lib/cpus/aarch64/cortex_a35.S \
				plat/common/plat_psci_common.c		\
				drivers/nuvoton/common/uart/nuvoton_16550_console.S	\
				plat/nuvoton/npcm845x/npcm845x_psci.c		\
				plat/nuvoton/npcm845x/npcm845x_serial_port.c \
				plat/nuvoton/common/nuvoton_topology.c		\
				plat/nuvoton/npcm845x/npcm845x_bl31_setup.c		
				

PLAT_BL_COMMON_SOURCES	:=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c		\
				plat/nuvoton/common/plat_nuvoton_gic.c	\
				${NPCM850_GIC_SOURCES}				\
				plat/nuvoton/npcm845x/npcm845x_common.c	\
				plat/nuvoton/common/nuvoton_helpers.S	\
				lib/semihosting/semihosting.c \
				lib/semihosting/${ARCH}/semihosting_call.S \
				
				
				
				
ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/${ARCH}/xlat_tables.c
else
include lib/xlat_tables_v2/xlat_tables.mk


endif

ifeq (${ENABLE_PMF}, 1)
ifeq (${ARCH}, aarch64)
BL31_SOURCES		+=	plat/arm/common/aarch64/execution_state_switch.c\
				plat/arm/common/arm_sip_svc.c			\				
				lib/pmf/pmf_smc.c
else
BL32_SOURCES		+=	plat/arm/common/arm_sip_svc.c			\
				lib/pmf/pmf_smc.c
endif
endif

ifeq (${SPD},spmd)
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c	\
				common/fdt_wrappers.c			\
				${LIBFDT_SRCS}

endif


ifeq (${EL3_EXCEPTION_HANDLING},1)
BL31_SOURCES		+=	plat/arm/common/aarch64/arm_ehf.c
endif

include plat/nuvoton/common/nuvoton_common.mk
BL1_SOURCES	:=
BL2_SOURCES	:=
BL2U_SOURCES :=



IMX_DEBUG_UART		?= 	0
$(eval $(call add_define,IMX_USE_UART${IMX_DEBUG_UART}))

DEBUG_CONSOLE		?= 	0
$(eval $(call add_define,DEBUG_CONSOLE))

$(eval $(call add_define,ARM_TSP_RAM_LOCATION_ID))


