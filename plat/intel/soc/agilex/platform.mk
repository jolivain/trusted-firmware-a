#
# Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
# Copyright (c) 2019, Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
PLAT_INCLUDES		:=	\
			-Iplat/intel/soc/agilex/			\
			-Iplat/intel/soc/agilex/include/		\
			-Iplat/intel/soc/common/drivers/

PLAT_BL_COMMON_SOURCES	:=	\
			lib/xlat_tables/xlat_tables_common.c 		\
			lib/xlat_tables/aarch64/xlat_tables.c 		\
			drivers/arm/gic/common/gic_common.c		\
			drivers/arm/gic/v2/gicv2_main.c			\
			drivers/arm/gic/v2/gicv2_helpers.c		\
			plat/common/plat_gicv2.c			\
			drivers/delay_timer/delay_timer.c		\
			drivers/delay_timer/generic_delay_timer.c  	\
			drivers/ti/uart/aarch64/16550_console.S		\
			plat/intel/soc/agilex/aarch64/platform_common.c \
			plat/intel/soc/agilex/aarch64/plat_helpers.S	\

BL2_SOURCES     +=	\
		drivers/partition/partition.c				\
		drivers/partition/gpt.c					\
		drivers/arm/pl061/pl061_gpio.c				\
		drivers/mmc/mmc.c					\
		drivers/synopsys/emmc/dw_mmc.c				\
		drivers/io/io_storage.c					\
		drivers/io/io_block.c					\
		drivers/io/io_fip.c					\
		drivers/gpio/gpio.c					\
		drivers/intel/soc/stratix10/io/s10_memmap_qspi.c	\
		plat/intel/soc/agilex/bl2_plat_setup.c			\
		plat/intel/soc/agilex/plat_storage.c			\
                plat/intel/soc/agilex/bl2_plat_mem_params_desc.c	\
		plat/intel/soc/agilex/soc/agilex_reset_manager.c	\
		plat/intel/soc/agilex/soc/agilex_handoff.c		\
		plat/intel/soc/agilex/soc/agilex_clock_manager.c	\
		plat/intel/soc/agilex/soc/agilex_pinmux.c		\
		plat/intel/soc/agilex/soc/agilex_memory_controller.c	\
		plat/intel/soc/agilex/plat_delay_timer.c		\
		lib/cpus/aarch64/cortex_a53.S				\
		plat/intel/soc/agilex/plat_image_load.c			\
		plat/intel/soc/agilex/soc/agilex_system_manager.c	\
		common/desc_image_load.c				\
		plat/intel/soc/agilex/soc/agilex_mailbox.c		\
		plat/intel/soc/common/drivers/qspi/cadence_qspi.c	\
		plat/intel/soc/common/drivers/wdt/watchdog.c		\
		plat/intel/soc/common/drivers/ccu/ncore_ccu.c

BL31_SOURCES	+=	\
		drivers/arm/cci/cci.c					\
		lib/cpus/aarch64/cortex_a53.S				\
		lib/cpus/aarch64/aem_generic.S				\
		lib/cpus/aarch64/cortex_a53.S				\
		plat/common/plat_psci_common.c				\
		plat/intel/soc/agilex/plat_sip_svc.c			\
		plat/intel/soc/agilex/bl31_plat_setup.c 		\
		plat/intel/soc/agilex/plat_psci.c			\
		plat/intel/soc/agilex/plat_topology.c			\
		plat/intel/soc/agilex/plat_delay_timer.c		\
		plat/intel/soc/agilex/soc/agilex_reset_manager.c	\
		plat/intel/soc/agilex/soc/agilex_pinmux.c		\
		plat/intel/soc/agilex/soc/agilex_clock_manager.c	\
		plat/intel/soc/agilex/soc/agilex_handoff.c		\
		plat/intel/soc/agilex/soc/agilex_mailbox.c

PROGRAMMABLE_RESET_ADDRESS	:= 0
BL2_AT_EL3			:= 1
MULTI_CONSOLE_API		:= 1
USE_COHERENT_MEM		:= 1
